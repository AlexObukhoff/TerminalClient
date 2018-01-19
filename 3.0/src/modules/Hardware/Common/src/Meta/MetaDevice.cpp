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
	mConfigurationGuard(QReadWriteLock::Recursive),
	mDeviceName(CMetaDevice::DefaultName),
	mLogDate(QDate::currentDate()),
	mOperatorPresence(false),
	mDetectingPosition(0),
	mInitialized(ERequestStatus::Fail),
	mLog(nullptr),
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
void MetaDevice::setConfigParameter(const QString & aName, const QVariant & aValue)
{
	QWriteLocker lock(&mConfigurationGuard);

	mConfiguration.insert(aName, aValue);
}

//--------------------------------------------------------------------------------
QVariant MetaDevice::getConfigParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration.value(aName);
}

//--------------------------------------------------------------------------------
QVariant MetaDevice::getConfigParameter(const QString & aName, const QVariant & aDefault) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration.value(aName, aDefault);
}

//--------------------------------------------------------------------------------
void MetaDevice::removeConfigParameter(const QString & aName)
{
	QWriteLocker lock(&mConfigurationGuard);

	mConfiguration.remove(aName);
}

//--------------------------------------------------------------------------------
bool MetaDevice::containsConfigParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration.contains(aName);
}

//--------------------------------------------------------------------------------
void MetaDevice::setDeviceParameter(const QString & aName, const QVariant & aValue, const QString & aExtensibleName)
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
bool MetaDevice::containsDeviceParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mDeviceData.contains(aName) && !mDeviceData.value(aName).isEmpty();
}

//---------------------------------------------------------------------------
void MetaDevice::logDeviceData(const SLogData & aData) const
{
	toLog(LogLevel::Normal, "Plugin path: " + getConfigParameter(CHardware::PluginPath).toString());

	if (!aData.pluginConfig.isEmpty())
	{
		toLog(LogLevel::Normal, "Plugin data:" + aData.pluginConfig);
	}

	if (!aData.device.isEmpty())
	{
		toLog(LogLevel::Normal, "Device data:" + aData.device);
	}

	QReadLocker lock(&mConfigurationGuard);

	if (mConfiguration[CHardwareSDK::RequiredDevice].value<IDevice *>() && !aData.requiedDevicePluginConfig.isEmpty())
	{
		toLog(LogLevel::Normal, "Requied device data:" + aData.requiedDevicePluginConfig);
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

		result.requiedDevicePluginConfig = getPartDeviceData(data);
	}

	TDeviceData data;
	QStringList names = getConfigParameter(CHardware::PluginParameterNames).toStringList();

	foreach(const QString & name, names)
	{
		QString key = name.toLower().replace(ASCII::Space, ASCII::Underscore);
		data.insert(key, getConfigParameter(key).toString());
	}

	result.pluginConfig = getPartDeviceData(data);
	result.device = getPartDeviceData(mDeviceData, false);

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
QVariant MetaDevice::getDeviceParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mDeviceData.value(aName);
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
void MetaDevice::toLog(LogLevel::Enum aLevel, const QString & aMessage) const
{
	if (mLog)
	{
		mLog->write(aLevel, aMessage);
	}
	else
	{
		qCritical("Log pointer is empty. Message:%s.", aMessage.toLocal8Bit().data());
	}
}

//--------------------------------------------------------------------------------
