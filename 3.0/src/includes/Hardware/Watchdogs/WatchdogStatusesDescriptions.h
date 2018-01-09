/* @file Описатель кодов состояний модемов. */

#pragma once

#include "Hardware/Common/BaseStatusDescriptions.h"
#include "Hardware/Watchdogs/WatchdogStatusCodes.h"

//--------------------------------------------------------------------------------
namespace WatchdogStatusCode
{
	#define ADD_WARNING(aStatusCode, aTranslation) append(Warning::aStatusCode, SStatusCodeSpecification(SDK::Driver::EWarningLevel::Warning, #aStatusCode, aTranslation))
	#define ADD_ERROR(aStatusCode, aTranslation) append(Error::aStatusCode, SStatusCodeSpecification(SDK::Driver::EWarningLevel::Error, #aStatusCode, aTranslation))

	class CSpecifications: public DeviceStatusCode::CSpecifications
	{
	public:
		CSpecifications()
		{
			ADD_WARNING(Door,      QCoreApplication::translate("WatchdogSensors", "#door"));
			ADD_WARNING(Safe,      QCoreApplication::translate("WatchdogSensors", "#safe"));
			ADD_WARNING(UpperUnit, QCoreApplication::translate("WatchdogSensors", "#upper_unit"));
			ADD_WARNING(LowerUnit, QCoreApplication::translate("WatchdogSensors", "#lower_unit"));
			ADD_WARNING(Kick,      QCoreApplication::translate("WatchdogSensors", "#kick"));
			ADD_WARNING(Tilt,      QCoreApplication::translate("WatchdogSensors", "#tilt"));
			ADD_WARNING(Power,     QCoreApplication::translate("WatchdogSensors", "#power"));
			ADD_WARNING(UPSLowBattery, QCoreApplication::translate("WatchdogSensors", "#ups_low_battery"));

			ADD_ERROR(SensorBlock,     QCoreApplication::translate("WatchdogSensors", "#sensor_block"));
			ADD_ERROR(Temperature,     QCoreApplication::translate("WatchdogSensors", "#temperature"));
			ADD_ERROR(PCVoltageBlock,  QCoreApplication::translate("WatchdogSensors", "#pc_voltage_block"));
			ADD_ERROR(PCVoltage,       QCoreApplication::translate("WatchdogSensors", "#pc_voltage"));
			ADD_ERROR(UPSVoltageBlock, QCoreApplication::translate("WatchdogSensors", "#ups_voltage_block"));
			ADD_ERROR(UPSVoltage,      QCoreApplication::translate("WatchdogSensors", "#ups_voltage"));
		}
	};
}

//--------------------------------------------------------------------------------
