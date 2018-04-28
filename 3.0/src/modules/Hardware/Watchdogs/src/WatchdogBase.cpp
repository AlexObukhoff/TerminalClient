/* @file Базовый класс сторожевого устойства. */

// Modules
#include "Hardware/Watchdogs/WatchdogStatusesDescriptions.h"

// Project
#include "WatchdogBase.h"

//-----------------------------------------------------------------------------
WatchdogBase::WatchdogBase()
{
	mPingTimer.moveToThread(&mThread);

	connect(&mPingTimer, SIGNAL(timeout()), SLOT(onPing()));

	mPollingInterval = 5000;

	mIOMessageLogging = ELoggingType::ReadWrite;
	mSensorDisabledValue = false;
	setConfigParameter(CHardware::Watchdog::CanRegisterKey, false);
	setConfigParameter(CHardware::Watchdog::CanWakeUpPC, false);
	mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new WatchdogStatusCode::CSpecifications());
}

//----------------------------------------------------------------------------
bool WatchdogBase::updateParameters()
{
	setPingEnable(true);

	return true;
}

//--------------------------------------------------------------------------------
bool WatchdogBase::release()
{
	setPingEnable(false);

	return TWatchdogBase::release();
}

//-----------------------------------------------------------------------------
void WatchdogBase::setPingEnable(bool aEnabled)
{
	if (checkConnectionAbility() && mPingTimer.interval())
	{
		toLog(LogLevel::Normal, aEnabled ? "Ping is enabled." : "Pinging is disabled.");

		QMetaObject::invokeMethod(&mPingTimer, aEnabled ? "start" : "stop", Qt::QueuedConnection);
	}
}

//---------------------------------------------------------------------------
void WatchdogBase::cleanStatusCodes(TStatusCodes & aStatusCodes)
{
	bool needUpdateConfiguration = false;

	for (auto it = CWatchdogs::SensorData.data().begin(); it != CWatchdogs::SensorData.data().end(); ++it)
	{
		QString sensor = it.key();
		QString sensorValue = getConfigParameter(sensor, CHardwareSDK::Values::Auto).toString();

		if (containsConfigParameter(sensor) && (sensorValue == CHardwareSDK::Values::Auto))
		{
			needUpdateConfiguration = true;

			bool sensorActive = aStatusCodes.contains(it->statusCode) != mSensorDisabledValue;
			sensorValue = sensorActive ? CHardwareSDK::Values::Use : CHardwareSDK::Values::NotUse;

			setConfigParameter(sensor, sensorValue);
		}

		if (sensorValue != CHardwareSDK::Values::Use)
		{
			aStatusCodes.remove(it->statusCode);
		}
	}

	if (needUpdateConfiguration)
	{
		emit configurationChanged();
	}

	TWatchdogBase::cleanStatusCodes(aStatusCodes);
}

//--------------------------------------------------------------------------------
void WatchdogBase::emitStatusCodes(TStatusCollection & aStatusCollection, int aExtendedStatus)
{
	TWatchdogBase::emitStatusCodes(aStatusCollection, aExtendedStatus);

	TStatusCodes statusCodes = getStatusCodes(aStatusCollection);

	foreach(int statusCode, statusCodes)
	{
		auto it = std::find_if(CWatchdogs::SensorData.data().begin(), CWatchdogs::SensorData.data().end(),
			[&] (const CWatchdogs::SSensorData & aData) -> bool { return aData.statusCode == statusCode; });

		if (it != CWatchdogs::SensorData.data().end())
		{
			QString actionValue = getConfigParameter(it->action).toString();

			if ((actionValue == CHardwareSDK::Values::Use) && CWatchdogs::SensorActionData.data().contains(actionValue))
			{
				int extendedStatus = CWatchdogs::SensorActionData[actionValue];

				emitStatusCode(statusCode, extendedStatus);
			}
		}
	}
}

//----------------------------------------------------------------------------
