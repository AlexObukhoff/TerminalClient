/* @file Описатель кодов состояний диспенсера. */

#pragma once

#include "Hardware/Common/BaseStatusDescriptions.h"
#include "Hardware/Dispensers/DispenserStatusCodes.h"

//--------------------------------------------------------------------------------
namespace DispenserStatusCode
{
	class CSpecifications: public DeviceStatusCode::CSpecifications
	{
	public:
		CSpecifications()
		{
			ADD_OK_STATUS(SingleMode, "");
			ADD_OK_STATUS(Locked,     "");

			/// Предупреждения.
			ADD_WARNING_STATUS(Unit0NearEmpty, QCoreApplication::translate("DispenserStatuses", "#unit_0_near_empty"));
			ADD_WARNING_STATUS(Unit1NearEmpty, QCoreApplication::translate("DispenserStatuses", "#unit_1_near_empty"));
			ADD_WARNING_STATUS(Unit2NearEmpty, QCoreApplication::translate("DispenserStatuses", "#unit_2_near_empty"));
			ADD_WARNING_STATUS(Unit3NearEmpty, QCoreApplication::translate("DispenserStatuses", "#unit_3_near_empty"));
			ADD_WARNING_STATUS(AllUnitsNearEmpty, QCoreApplication::translate("DispenserStatuses", "#all_units_near_empty"));

			ADD_WARNING_STATUS(Unit0Empty, QCoreApplication::translate("DispenserStatuses", "#unit_0_empty"));
			ADD_WARNING_STATUS(Unit1Empty, QCoreApplication::translate("DispenserStatuses", "#unit_1_empty"));
			ADD_WARNING_STATUS(Unit2Empty, QCoreApplication::translate("DispenserStatuses", "#unit_2_empty"));
			ADD_WARNING_STATUS(Unit3Empty, QCoreApplication::translate("DispenserStatuses", "#unit_3_empty"));

			/// Ошибки.
			ADD_ERROR_STATUS(AllUnitsEmpty, QCoreApplication::translate("DispenserStatuses", "#all_units_empty"));
			ADD_ERROR_STATUS(Unit0Opened, QCoreApplication::translate("DispenserStatuses", "#unit_0_opened"));
			ADD_ERROR_STATUS(Unit1Opened, QCoreApplication::translate("DispenserStatuses", "#unit_1_opened"));
			ADD_ERROR_STATUS(Unit2Opened, QCoreApplication::translate("DispenserStatuses", "#unit_2_opened"));
			ADD_ERROR_STATUS(Unit3Opened, QCoreApplication::translate("DispenserStatuses", "#unit_3_opened"));
			ADD_ERROR_STATUS(RejectingOpened, QCoreApplication::translate("DispenserStatuses", "#rejecting_opened"));
			ADD_ERROR_STATUS(Jammed, QCoreApplication::translate("DispenserStatuses", "#jammed"));
		}
	};
}

//--------------------------------------------------------------------------------
