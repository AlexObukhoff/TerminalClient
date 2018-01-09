/* @file Сторожевой таймер ОСМП2. */

#pragma once

// Modules
#include "Hardware/Common/Specifications.h"

// Project
#include "OSMP.h"

//--------------------------------------------------------------------------------
namespace COSMP2Sensor
{
	struct SStatusCodeData
	{
		int statusCode;
		bool invert;
		int enable;

		SStatusCodeData() : statusCode(DeviceStatusCode::OK::OK), invert(false), enable(-1) {}
		SStatusCodeData(int aStatusCode, bool aInvert, int aEnable) : statusCode(aStatusCode), invert(aInvert), enable(aEnable) {}
	};

	/// Спецификация состояний сенсоров.
	class CSensorCodeSpecification : public CSpecification<int, SStatusCodeData>
	{
	public:
		CSensorCodeSpecification()
		{
			add(0, 6, WatchdogStatusCode::Warning::Door, false, 5);
			add(0, 3, WatchdogStatusCode::Error::SensorBlock, true);

			add(1, 3, WatchdogStatusCode::Error::Temperature, true, 4);
			add(1, 2, WatchdogStatusCode::Warning::UPSLowBattery, false);
			add(1, 1, WatchdogStatusCode::Error::UPSVoltage, false);
			add(1, 0, WatchdogStatusCode::Error::PCVoltage, true);

			add(2, 7, WatchdogStatusCode::Error::PCVoltageBlock, true);
			add(2, 6, WatchdogStatusCode::Error::UPSVoltageBlock, true);
		}

	private:
		void add(int aByte, int aBit, int aStatusCode, bool aInvert, int aEnable = -1)
		{
			append(aByte * 8 + aBit, SStatusCodeData(aStatusCode, aInvert, aEnable));
		}
	};

	static CSensorCodeSpecification SensorCodeSpecification;
}

//--------------------------------------------------------------------------------
class OSMP2: public OSMP
{
	SET_SERIES("OSMP2")

public:
	OSMP2();

protected:
	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Анализирует коды статусов устройства и фильтрует лишние.
	virtual void cleanStatusCodes(TStatusCodes & aStatusCodes);
};

//--------------------------------------------------------------------------------
