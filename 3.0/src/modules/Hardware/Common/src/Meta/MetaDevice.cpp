/* @file Базовый класс устройства. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtAlgorithms>
#include <QtCore/QWriteLocker>
#include <QtCore/QReadLocker>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/ICardReader.h>
#include <SDK/Drivers/ICashAcceptor.h>
#include <SDK/Drivers/IDevice.h>
#include <SDK/Drivers/IDispenser.h>
#include <SDK/Drivers/IFiscalPrinter.h>
#include <SDK/Drivers/IHID.h>
#include <SDK/Drivers/IIOPort.h>
#include <SDK/Drivers/IMifareReader.h>
#include <SDK/Drivers/IModem.h>
#include <SDK/Drivers/IPrinter.h>
#include <SDK/Drivers/IWatchdog.h>

// Project
#include "MetaDevice.h"

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
template class MetaDevice<ICardReader>;
template class MetaDevice<ICashAcceptor>;
template class MetaDevice<IDevice>;
template class MetaDevice<IDispenser>;
template class MetaDevice<IFiscalPrinter>;
template class MetaDevice<IHID>;
template class MetaDevice<IIOPort>;
template class MetaDevice<IMifareReader>;
template class MetaDevice<IModem>;
template class MetaDevice<IPrinter>;
template class MetaDevice<IWatchdog>;

//-------------------------------------------------------------------------------
template <class T>
MetaDevice<T>::MetaDevice() :
	mDeviceName(CMetaDevice::DefaultName),
	mLogDate(QDate::currentDate()),
	mOperatorPresence(false),
	mDetectingPosition(0),
	mInitialized(ERequestStatus::Fail),
	mExitTimeout(ULONG_MAX)
{
}

//--------------------------------------------------------------------------------
template <class T>
bool MetaDevice<T>::subscribe(const char * /*aSignal*/, QObject * /*aReceiver*/, const char * /*aSlot*/)
{
	return false;
}

//--------------------------------------------------------------------------------
template <class T>
bool MetaDevice<T>::unsubscribe(const char * /*aSignal*/, QObject * /*aReceiver*/)
{
	return false;
}

//--------------------------------------------------------------------------------
template <class T>
QString MetaDevice<T>::getName() const
{
	QString deviceName = getConfigParameter(CHardwareSDK::ModelName).toString();

	return deviceName.isEmpty() ? mDeviceName : deviceName;
}

//--------------------------------------------------------------------------------
template <class T>
void MetaDevice<T>::initialize()
{
	logDeviceData(getDeviceData());

	mInitialized = ERequestStatus::Success;
}

//--------------------------------------------------------------------------------
template <class T>
bool MetaDevice<T>::release()
{
	if (mThread.isRunning())
	{
		mThread.quit();

		if (!isWorkingThread())
		{
			mThread.wait(mExitTimeout);
		}
	}

	mInitialized = ERequestStatus::Fail;

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void MetaDevice<T>::updateFirmware(const QByteArray & /*aBuffer*/)
{
}

//--------------------------------------------------------------------------------
template <class T>
bool MetaDevice<T>::canUpdateFirmware()
{
	return false;
}

//--------------------------------------------------------------------------------
template <class T>
bool MetaDevice<T>::isAutoDetecting() const
{
	return getConfigParameter(CHardwareSDK::SearchingType).toString() == CHardwareSDK::SearchingTypes::AutoDetecting;
}

//--------------------------------------------------------------------------------
template <class T>
void MetaDevice<T>::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	for (auto it = aConfiguration.begin(); it != aConfiguration.end(); ++it)
	{
		setConfigParameter(it.key(), it.value());
	}

	mOperatorPresence = aConfiguration.value(CHardwareSDK::OperatorPresence, mOperatorPresence).toBool();
}

//--------------------------------------------------------------------------------
template <class T>
QVariantMap MetaDevice<T>::getDeviceConfiguration() const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration;
}

//--------------------------------------------------------------------------------
template <class T>
void MetaDevice<T>::setDeviceParameter(const QString & aName, const QVariant & aValue, const QString & aExtensibleName, bool aUpdateExtensible)
{
	QString value = aValue.toString().simplified();
	QVariant::Type type = aValue.type();

	if (type == QVariant::ByteArray)
	{
		value = ProtocolUtils::clean(aValue.toByteArray());
	}
	else if (type == QVariant::String)
	{
		value = ProtocolUtils::clean(aValue.toString());
	}
	else if (type == QVariant::Bool)
	{
		value = aValue.toBool() ? CDeviceData::Values::Yes : CDeviceData::Values::No;
	}

	if (aExtensibleName.isEmpty())
	{
		QWriteLocker locker(&mConfigurationGuard);

		mDeviceData.insert(aName, value);
	}
	else if (!value.isEmpty())
	{
		value.prepend(aName + " ");

		if (!aUpdateExtensible)
		{
			QReadLocker locker(&mConfigurationGuard);

			QString extensibleValue = mDeviceData[aExtensibleName];

			if (!extensibleValue.isEmpty())
			{
				value.prepend(extensibleValue + ", ");
			}
		}

		{
			QWriteLocker locker(&mConfigurationGuard);

			mDeviceData.insert(aExtensibleName, value);
		}
	}
}

//--------------------------------------------------------------------------------
template <class T>
QVariant MetaDevice<T>::getDeviceParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mDeviceData.value(aName);
}

//--------------------------------------------------------------------------------
template <class T>
bool MetaDevice<T>::containsDeviceParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mDeviceData.contains(aName) && !mDeviceData.value(aName).isEmpty();
}

//--------------------------------------------------------------------------------
template <class T>
void MetaDevice<T>::removeDeviceParameter(const QString & aName)
{
	QWriteLocker lock(&mConfigurationGuard);

	mDeviceData.remove(aName);
}

//---------------------------------------------------------------------------
template <class T>
void MetaDevice<T>::logDeviceData(const SLogData & aData) const
{
	toLog(LogLevel::Normal, "Plugin path: " + getConfigParameter(CHardware::PluginPath).toString());

	if (!aData.plugin.isEmpty()) toLog(LogLevel::Normal, "Plugin data:" + aData.plugin);
	if (!aData.device.isEmpty()) toLog(LogLevel::Normal, "Device data:" + aData.device);
	if (!aData.config.isEmpty()) toLog(LogLevel::Normal, "Config data:" + aData.config);

	QReadLocker lock(&mConfigurationGuard);

	if (mConfiguration[CHardwareSDK::RequiredDevice].value<IDevice *>() && !aData.requiedDevice.isEmpty())
	{
		toLog(LogLevel::Normal, "Requied device data:" + aData.requiedDevice);
	}
}

//---------------------------------------------------------------------------
template <class T>
SLogData MetaDevice<T>::getDeviceData() const
{
	QReadLocker lock(&mConfigurationGuard);

	IDevice * requiredDevice = mConfiguration.value(CHardwareSDK::RequiredDevice).value<IDevice *>();
	SLogData result;

	if (requiredDevice)
	{
		TDeviceData data;
		QStringList names = getConfigParameter(CHardware::RequiredResourceNames).toStringList();
		QVariantMap configuration = requiredDevice->getDeviceConfiguration();

		foreach(const QString & name, names)
		{
			QString key = name.toLower().replace(ASCII::Space, ASCII::Underscore);
			data.insert(key, configuration[key].toString());
		}

		result.requiedDevice = getPartDeviceData(data);
	}

	TDeviceData data;
	QStringList names = getConfigParameter(CHardware::PluginParameterNames).toStringList();

	foreach(const QString & name, names)
	{
		QString key = name.toLower().replace(ASCII::Space, ASCII::Underscore);
		data.insert(key, getConfigParameter(key).toString());
	}

	result.plugin = getPartDeviceData(data, false);
	result.device = getPartDeviceData(mDeviceData, false);

	QVariantMap dealerSettings = getConfigParameter(CHardware::ConfigData).toMap();
	names = dealerSettings.keys();
	data.clear();

	foreach(const QString & name, names)
	{
		QString key = name.toLower().replace(ASCII::Space, ASCII::Underscore);
		data.insert(key, dealerSettings[key].toString());
	}

	result.config = getPartDeviceData(data);

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
QString MetaDevice<T>::getPartDeviceData(const TDeviceData & aData, bool aHideEmpty) const
{
	QStringList keys = aData.keys();
	int maxSize = 0;

	foreach(auto key, keys)
	{
		maxSize = qMax(maxSize, key.size());
	}

	keys.sort();
	QString result;

	for (int i = 0; i < keys.size(); ++i)
	{
		QString key = keys[i];
		QString value = aData[key];

		if (!aHideEmpty || !value.isEmpty())
		{
			key += QString(maxSize - key.size(), QChar(ASCII::Space));
			result += QString("\n%1 : %2").arg(key).arg(value);
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
IDevice::IDetectingIterator * MetaDevice<T>::getDetectingIterator()
{
	mDetectingPosition = 0;

	return this;
}

//--------------------------------------------------------------------------------
template <class T>
bool MetaDevice<T>::find()
{
	return false;
}

//--------------------------------------------------------------------------------
template <class T>
bool MetaDevice<T>::moveNext()
{
	return (mDetectingPosition++ == 0);
}

//--------------------------------------------------------------------------------
template <class T>
void MetaDevice<T>::setLog(ILog * aLog)
{
	mLog = aLog;
}

//--------------------------------------------------------------------------------
template <class T>
bool MetaDevice<T>::isWorkingThread()
{
	return &mThread == QThread::currentThread();
}

//--------------------------------------------------------------------------------
