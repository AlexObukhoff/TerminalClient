/* @file Сканер Honeywell Metrologic на OPOS-драйвере. */

#import "OPOSScanner.tlb"

// Windows
#include <objbase.h>

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QUuid>
#include <QtCore/QRegExp>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/BaseStatusDescriptions.h"

// Project
#include "OPOSMetrologicScanner.h"

#pragma pop_macro("max")
#pragma pop_macro("min")

//--------------------------------------------------------------------------------
OPOSMetrologicScanner::OPOSMetrologicScanner()
{
	mDeviceName = "Honeywell Metrologic based on OPOS driver";
	mClaimTimeout = 2000;
	mProfileNames = getProfileNames();
	mPollingInterval = 500;
}

//--------------------------------------------------------------------------------
bool OPOSMetrologicScanner::isConnected()
{
	if (!TPollingOPOSScanner::isConnected())
	{
		return false;
	}

	QString deviceName = getConfigParameter(CHardwareSDK::ModelName).toString();
	deviceName = deviceName.replace("/", " ").replace(QRegExp(" +"), " ");

	if (!deviceName.isEmpty())
	{
		mDeviceName = deviceName;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool OPOSMetrologicScanner::checkConnectionAbility()
{
	if (!TPollingOPOSScanner::checkConnectionAbility())
	{
		return false;
	}

	typedef OposScanner_1_8_Lib::IOPOSScanner TNativeDriver;
	TNativeDriver * nativeDriver;

	if (FAILED(mDriver->queryInterface(QUuid(__uuidof(TNativeDriver)), (void**)&nativeDriver)))
	{
		toLog(LogLevel::Error, "Failed to query interface of the scanner.");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool OPOSMetrologicScanner::updateParameters()
{
	return setAvailable(true);
}

//--------------------------------------------------------------------------------
void OPOSMetrologicScanner::initializeResources()
{
	TPollingOPOSScanner::initializeResources();

	if (mCOMInitialized && !mDriver.isNull())
	{
		connect(mDriver.data(), SIGNAL(signal(const QString &, int, void *)), SLOT(onGotData(const QString &, int, void *)), Qt::UniqueConnection);
	}
}

//--------------------------------------------------------------------------------
bool OPOSMetrologicScanner::enable(bool aReady)
{
	if (!mStatusCollectionHistory.isEmpty() && (!checkConnectionAbility() && (mInitialized == ERequestStatus::Fail)))
	{
		toLog(LogLevel::Error, QString("%1: Cannot set possibility of data receiving to %2").arg(mDeviceName).arg(aReady ? "true" : "false"));
		return !aReady;
	}

	if (!isWorkingThread() || (mInitialized == ERequestStatus::InProcess))
	{
		QMetaObject::invokeMethod(this, "enable", Qt::BlockingQueuedConnection, Q_ARG(bool, aReady));
	}
	else 
	{
		bool claimed = BOOL_CALL_OPOS(Claimed);
		bool enabled = BOOL_CALL_OPOS(DeviceEnabled);

		if (!claimed || !enabled)
		{
			toLog(aReady ? LogLevel::Error : LogLevel::Normal, QString("%1: device is not %2, setEnable returns %3")
				.arg(mDeviceName).arg(enabled ? "claimed" : "enabled").arg(aReady ? "false" : "true"));
			return !aReady;
		}

		toLog(LogLevel::Normal, QString("%1: set possibility of data receiving to %2").arg(mDeviceName).arg(aReady ? "true" : "false"));

		VOID_CALL_OPOS(SetDataEventEnabled, aReady);
		VOID_CALL_OPOS(SetDecodeData, aReady);
	}

	return (BOOL_CALL_OPOS(DataEventEnabled) == aReady) && (BOOL_CALL_OPOS(DecodeData) == aReady);
}

//--------------------------------------------------------------------------------
bool OPOSMetrologicScanner::setAvailable(bool aEnable)
{
	QString log = aEnable ? "enabled" : "disabled";
	toLog(LogLevel::Normal, QString("%1: set scanner %2").arg(mDeviceName).arg(log));

	if (!BOOL_CALL_OPOS(Claimed))
	{
		toLog(aEnable ? LogLevel::Error : LogLevel::Normal, QString("%1: device is not claimed, setAvailable returns %2").arg(mDeviceName).arg(aEnable ? "false" : "true"));
		return !aEnable;
	}

	if (aEnable == BOOL_CALL_OPOS(DeviceEnabled))
	{
		toLog(LogLevel::Normal, QString("%1: already %2").arg(mDeviceName).arg(log));
		return true;
	}

	VOID_CALL_OPOS(SetDeviceEnabled, aEnable);

	if (aEnable != BOOL_CALL_OPOS(DeviceEnabled))
	{
		toLog(LogLevel::Error, QString("%1: Failed to set device %2").arg(mDeviceName).arg(log));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
void OPOSMetrologicScanner::onGotData(const QString & /*aName*/, int /*aArgumentsCount*/, void * /*aArgumentsValues*/)
{
	QMutexLocker lock(&mDataMutex);

	QString capturedData = (((OPOS::OPOSScanner *)sender())->ScanData()).trimmed();

	QString logData = capturedData;

	for (char ch = 0x00; ch < 0x20; ++ch)
	{
		capturedData.replace(ch, "");
	}

	QString log = QString("%1: data received: %2").arg(getName()).arg(capturedData);

	if (logData != capturedData)
	{
		log += QString(", {%1} -> {%2}").arg(logData.toLatin1().toHex().data()).arg(capturedData.toLatin1().toHex().data());
	}

	if (getConfigParameter(CHardware::Scanner::Prefix).toBool())
	{
		capturedData = capturedData.mid(COPOSScanners::Prefix);
		log += QString(" -> {%1}").arg(capturedData.toLatin1().toHex().data());
	}

	toLog(LogLevel::Normal, log);

	((OPOS::OPOSScanner *)sender())->SetDataEventEnabled(true);

	QVariantMap result;
	result[CHardwareSDK::HID::Text] = capturedData;

	emit data(result);
}

//--------------------------------------------------------------------------------
QStringList OPOSMetrologicScanner::getProfileNames()
{
	static QStringList result;

	if (result.isEmpty())
	{
		result = QSettings("SOFTWARE\\OLEforRetail\\ServiceOPOS\\Scanner", QSettings::NativeFormat).childGroups();
	}

	return result;
}

//--------------------------------------------------------------------------------
