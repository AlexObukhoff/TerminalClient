/* @file Описатель кодов состояний монитора состояний компьютера. */

#pragma once

#include "Hardware/Common/BaseStatusDescriptions.h"
#include "Hardware/Health/HealthStatusCodes.h"

//--------------------------------------------------------------------------------
namespace HealthStatusCode
{
	class CSpecifications: public DeviceStatusCode::CSpecifications
	{
	public:
		CSpecifications()
		{
			/// Предупреждения.
			ADD_WARNING_STATUS(HDD0NearEnd,     QCoreApplication::translate("HealthStatuses", "#hdd0_near_end"));
			ADD_WARNING_STATUS(HDD0Overheating, QCoreApplication::translate("HealthStatuses", "#hdd0_overheating"));
			ADD_WARNING_STATUS(HDD0NearDead,    QCoreApplication::translate("HealthStatuses", "#hdd0_near_dead"));
			ADD_WARNING_STATUS(HDD0Worsened,    QCoreApplication::translate("HealthStatuses", "#hdd0_worsened"));

			ADD_WARNING_STATUS(HDD1NearEnd,     QCoreApplication::translate("HealthStatuses", "#hdd1_near_end"));
			ADD_WARNING_STATUS(HDD1Overheating, QCoreApplication::translate("HealthStatuses", "#hdd1_overheating"));
			ADD_WARNING_STATUS(HDD1NearDead,    QCoreApplication::translate("HealthStatuses", "#hdd1_near_dead"));
			ADD_WARNING_STATUS(HDD1Worsened,    QCoreApplication::translate("HealthStatuses", "#hdd1_worsened"));

			ADD_WARNING_STATUS(HDD2NearEnd,     QCoreApplication::translate("HealthStatuses", "#hdd2_near_end"));
			ADD_WARNING_STATUS(HDD2Overheating, QCoreApplication::translate("HealthStatuses", "#hdd2_overheating"));
			ADD_WARNING_STATUS(HDD2NearDead,    QCoreApplication::translate("HealthStatuses", "#hdd2_near_dead"));
			ADD_WARNING_STATUS(HDD2Worsened,    QCoreApplication::translate("HealthStatuses", "#hdd2_worsened"));

			ADD_WARNING_STATUS(HDD3NearEnd,     QCoreApplication::translate("HealthStatuses", "#hdd3_near_end"));
			ADD_WARNING_STATUS(HDD3Overheating, QCoreApplication::translate("HealthStatuses", "#hdd3_overheating"));
			ADD_WARNING_STATUS(HDD3NearDead,    QCoreApplication::translate("HealthStatuses", "#hdd3_near_dead"));
			ADD_WARNING_STATUS(HDD3Worsened,    QCoreApplication::translate("HealthStatuses", "#hdd3_worsened"));

			ADD_WARNING_STATUS(MemoryNearEnd,      QCoreApplication::translate("HealthStatuses", "#memory_near_end"));
			ADD_WARNING_STATUS(SRNearEnd,          QCoreApplication::translate("HealthStatuses", "#system_resources_near_end"));
			ADD_WARNING_STATUS(AllSRNearEnd,       QCoreApplication::translate("HealthStatuses", "#all_system_resources_near_end"));
			ADD_WARNING_STATUS(CPUOverheating,     QCoreApplication::translate("HealthStatuses", "#cpu_overheating"));
			ADD_WARNING_STATUS(OSVersion,          QCoreApplication::translate("HealthStatuses", "#unsupported_os_version"));
			ADD_WARNING_STATUS(NeedChangeTimezone, QCoreApplication::translate("HealthStatuses", "#need_change_timezone"));
			ADD_WARNING_STATUS(NeedConfigTimezone, QCoreApplication::translate("HealthStatuses", "#need_config_timezone"));
			ADD_WARNING_STATUS(Antivirus,          QCoreApplication::translate("HealthStatuses", "#antivirus_disabled"));
			ADD_WARNING_STATUS(Firewall,           QCoreApplication::translate("HealthStatuses", "#firewall_disabled"));

			/// Ошибки.
			ADD_ERROR_STATUS(HDD0End,        QCoreApplication::translate("HealthStatuses", "#hdd0_end"));
			ADD_ERROR_STATUS(HDD0Overheated, QCoreApplication::translate("HealthStatuses", "#hdd0_overheated"));
			ADD_ERROR_STATUS(HDD0Dead,       QCoreApplication::translate("HealthStatuses", "#hdd0_dead"));

			ADD_ERROR_STATUS(HDD1End,        QCoreApplication::translate("HealthStatuses", "#hdd1_end"));
			ADD_ERROR_STATUS(HDD1Overheated, QCoreApplication::translate("HealthStatuses", "#hdd1_overheated"));
			ADD_ERROR_STATUS(HDD1Dead,       QCoreApplication::translate("HealthStatuses", "#hdd1_dead"));

			ADD_ERROR_STATUS(HDD2End,        QCoreApplication::translate("HealthStatuses", "#hdd2_end"));
			ADD_ERROR_STATUS(HDD2Overheated, QCoreApplication::translate("HealthStatuses", "#hdd2_overheated"));
			ADD_ERROR_STATUS(HDD2Dead,       QCoreApplication::translate("HealthStatuses", "#hdd2_dead"));

			ADD_ERROR_STATUS(HDD3End,        QCoreApplication::translate("HealthStatuses", "#hdd3_end"));
			ADD_ERROR_STATUS(HDD3Overheated, QCoreApplication::translate("HealthStatuses", "#hdd3_overheated"));
			ADD_ERROR_STATUS(HDD3Dead,       QCoreApplication::translate("HealthStatuses", "#hdd3_dead"));

			ADD_ERROR_STATUS(SREnd,          QCoreApplication::translate("HealthStatuses", "#system_resources_end"));
			ADD_ERROR_STATUS(AllSREnd,       QCoreApplication::translate("HealthStatuses", "#all_system_resources_end"));
			ADD_ERROR_STATUS(MemoryEnd,      QCoreApplication::translate("HealthStatuses", "#memory_end"));
			ADD_ERROR_STATUS(CPUOverheated,  QCoreApplication::translate("HealthStatuses", "#cpu_overheated"));
		}
	};
}

//--------------------------------------------------------------------------------
