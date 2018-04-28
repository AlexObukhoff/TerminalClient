/* @file Базовый класс устройства. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtAlgorithms>
#include <QtCore/QWriteLocker>
#include <QtCore/QReadLocker>
#include <Common/QtHeadersEnd.h>

// Project
#include "MetaDevice.h"

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
MetaDevice::MetaDevice() :
	mDeviceName(CMetaDevice::DefaultName),
	mLogDate(QDate::currentDate()),
	mOperatorPresence(false),
	mDetectingPosition(0),
	mInitialized(ERequestStatus::Fail),
	mExitTimeout(ULONG_MAX)
{
}

//--------------------------------------------------------------------------------
bool MetaDevice::subscribe(const char * /*aSignal*/, QObject * /*aReceiver*/, const char * /*aSlot*/)
{
	return false;
}

//--------------------------------------------------------------------------------
bool MetaDevice::unsubscribe(const char * /*aSignal*/, QObject * /*aReceiver*/)
{
	return false;
}

//--------------------------------------------------------------------------------
QString MetaDevice::getName() const
{
	QString deviceName = getConfigParameter(CHardwareSDK::ModelName).toString();

	return deviceName.isEmpty() ? mDeviceName : deviceName;
}

//--------------------------------------------------------------------------------
void MetaDevice::initialize()
{
	logDeviceData(getDeviceData());

	mInitialized = ERequestStatus::Success;
}

//--------------------------------------------------------------------------------
bool MetaDevice::release()
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
void MetaDevice::updateFirmware(const QByteArray & /*aBuffer*/)
{
}

//--------------------------------------------------------------------------------
bool MetaDevice::canUpdateFirmware()
{
	return false;
}

//--------------------------------------------------------------------------------
bool MetaDevice::isAutoDetecting() const
{
	return getConfigParameter(CHardwareSDK::SearchingType).toString() == CHardwareSDK::SearchingTypes::AutoDetecting;
}

//--------------------------------------------------------------------------------
void MetaDevice::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	for (auto it = aConfiguration.begin(); it != aConfiguration.end(); ++it)
	{
		setConfigParameter(it.key(), it.value());
	}

	mOperatorPresence = aConfiguration.value(CHardwareSDK::OperatorPresence, mOperatorPresence).toBool();
}

//--------------------------------------------------------------------------------
QVariantMap MetaDevice::getDeviceConfiguration() const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration;
}

//--------------------------------------------------------------------------------
void MetaDevice::setDeviceParameter(const QString & aName, const QVariant & aValue, const QString & aExtensibleName, bool aUpdateExtensible)
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
QVariant MetaDevice::getDeviceParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mDeviceData.value(aName);
}

//--------------------------------------------------------------------------------
bool MetaDevice::containsDeviceParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mDeviceData.contains(aName) && !mDeviceData.value(aName).isEmpty();
}

//--------------------------------------------------------------------------------
void MetaDevice::removeDeviceParameter(const QString & aName)
{
	QWriteLocker lock(&mConfigurationGuard);

	mDeviceData.remove(aName);
}

//---------------------------------------------------------------------------
void MetaDevice::logDeviceData(const SLogData & aData) const
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
SLogData MetaDevice::getDeviceData() const
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
QString MetaDevice::getPartDeviceData(const TDeviceData & aData, bool aHideEmpty) const
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
IDevice::IDetectingIterator * MetaDevice::getDetectingIterator()
{
	mDetectingPosition = 0;

	return this;
}

//--------------------------------------------------------------------------------
bool MetaDevice::find()
{
	return false;
}

//--------------------------------------------------------------------------------
bool MetaDevice::moveNext()
{
	return (mDetectingPosition++ == 0);
}

//--------------------------------------------------------------------------------
void MetaDevice::setLog(ILog * aLog)
{
	mLog = aLog;
}

//--------------------------------------------------------------------------------
bool MetaDevice::isWorkingThread()
{
	return &mThread == QThread::currentThread();
}

//--------------------------------------------------------------------------------
