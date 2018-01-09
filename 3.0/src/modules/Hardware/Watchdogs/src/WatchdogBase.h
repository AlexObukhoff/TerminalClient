/* @file Базовый класс сторожевого устойства. */

#pragma once

// SDK
#include <SDK/Drivers/Watchdog/LineTypes.h>
#include "SDK/Drivers/Watchdog/WatchdogStatus.h"

// Modules
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"

// Project
#include "Hardware/Watchdogs/ProtoWatchdog.h"

namespace CWatchdogs
{
	struct SSensorData
	{
		int statusCode;
		QString action;

		SSensorData() {}
		SSensorData(int aStatusCode, const QString & aAction) : statusCode(aStatusCode), action(aAction) {}
	};

	class CSensorData: public CSpecification<QString, SSensorData>
	{
	public:
		CSensorData()
		{
			#define ADD_SENSOR_DATA(aSensor) append(CHardware::Watchdog::Sensor::aSensor, SSensorData(WatchdogStatusCode::Warning::aSensor, CHardware::Watchdog::Sensor::Action::aSensor))

			ADD_SENSOR_DATA(Safe);
			ADD_SENSOR_DATA(UpperUnit);
			ADD_SENSOR_DATA(LowerUnit);
		}
	};

	static CSensorData SensorData;

	class CSensorActionData: public CSpecification<QString, SDK::Driver::EWatchdogStatus::Enum>
	{
	public:
		CSensorActionData()
		{
			#define ADD_SENSOR_ACTION(aAction) append(CHardware::Watchdog::Sensor::ActionValue::aAction, SDK::Driver::EWatchdogStatus::aAction)

			ADD_SENSOR_ACTION(EnterServiceMenu);
			ADD_SENSOR_ACTION(LockTerminal);
		}
	};

	static CSensorActionData SensorActionData;
}

//----------------------------------------------------------------------------
typedef SerialDeviceBase<PortPollingDeviceBase<ProtoWatchdog>> TWatchdogBase;

class WatchdogBase : public TWatchdogBase
{
public:
	WatchdogBase();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
	virtual bool release();

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Анализирует коды статусов устройства и фильтрует лишние.
	virtual void cleanStatusCodes(TStatusCodes & aStatusCodes);

	/// Отправить статус-коды.
	virtual void emitStatusCodes(TStatusCollection & aStatusCollection, int aExtendedStatus = SDK::Driver::EStatus::Actual);

	/// Запуск/останов пинга.
	virtual void setPingEnable(bool aEnabled);

	/// Таймер для сброса внутреннего таймера датчика.
	QTimer mPingTimer;

	/// Показания датчиков в неподключенном состоянии.
	bool mSensorDisabledValue;

	/// Время включения PC.
	QTime mPCWakingUpTime;
};

//----------------------------------------------------------------------------
