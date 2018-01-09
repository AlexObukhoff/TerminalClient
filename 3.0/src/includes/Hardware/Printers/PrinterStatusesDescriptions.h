/* @file Описатель кодов состояний принтеров. */

#pragma once

#include "Hardware/Common/BaseStatusDescriptions.h"
#include "Hardware/Printers/PrinterStatusCodes.h"

#define ADD_PRINTER_STATUS(aStatusCode, aTranslation, aWarningLevel) ADD_BASE_STATUS(aStatusCode, aTranslation, aWarningLevel); mStatusCodes.insert(aWarningLevel::aStatusCode)
#define ADD_PRINTER_WARNING(aStatusCode, aTranslation) ADD_PRINTER_STATUS(aStatusCode, aTranslation, Warning)
  #define ADD_PRINTER_ERROR(aStatusCode, aTranslation) ADD_PRINTER_STATUS(aStatusCode, aTranslation, Error)

//--------------------------------------------------------------------------------
namespace PrinterStatusCode
{
	class CSpecifications: public DeviceStatusCode::CSpecifications
	{
	public:
		CSpecifications()
		{
			/// OK.
			ADD_OK_STATUS(PaperInPresenter, QCoreApplication::translate("PrinterStatuses", "#paper_in_presenter"));
			ADD_OK_STATUS(MotorMotion,      QCoreApplication::translate("PrinterStatuses", "#motor_motion"));

			/// Предупреждения.
			ADD_PRINTER_WARNING(PaperNearEnd,         QCoreApplication::translate("PrinterStatuses", "#paper_near_end"));
			ADD_PRINTER_WARNING(ControlPaperNearEnd,  QCoreApplication::translate("PrinterStatuses", "#control_paper_near_end"));
			ADD_PRINTER_WARNING(TonerNearEnd,         QCoreApplication::translate("PrinterStatuses", "#toner_near_end"));
			ADD_PRINTER_WARNING(PaperEndVirtual,      QCoreApplication::translate("PrinterStatuses", "#no_paper_virtual"));
			ADD_PRINTER_WARNING(WrongFWConfiguration, QCoreApplication::translate("PrinterStatuses", "#wrong_firmware_configuration"));

			/// Ошибки.
			ADD_PRINTER_ERROR(PaperEnd,              QCoreApplication::translate("PrinterStatuses", "#no_paper"));
			ADD_PRINTER_ERROR(ControlPaperEnd,       QCoreApplication::translate("PrinterStatuses", "#no_control_paper"));
			ADD_PRINTER_ERROR(PaperJam,              QCoreApplication::translate("PrinterStatuses", "#paper_is_jammed"));
			ADD_PRINTER_ERROR(Paper,                 QCoreApplication::translate("PrinterStatuses", "#paper_error"));
			ADD_PRINTER_ERROR(Temperature,           QCoreApplication::translate("PrinterStatuses", "#printer_head_overheating"));
			ADD_PRINTER_ERROR(PrintingHead,          QCoreApplication::translate("PrinterStatuses", "#printing_head_error"));
			ADD_PRINTER_ERROR(Cutter,                QCoreApplication::translate("PrinterStatuses", "#cutter_error"));
			ADD_PRINTER_ERROR(Presenter,             QCoreApplication::translate("PrinterStatuses", "#presenter_error"));
			ADD_PRINTER_ERROR(Port,                  QCoreApplication::translate("PrinterStatuses", "#port_error"));
			ADD_PRINTER_ERROR(PrinterFR,             QCoreApplication::translate("PrinterStatuses", "#printer_fr_error"));
			ADD_PRINTER_ERROR(PrinterFRCollapse,     QCoreApplication::translate("PrinterStatuses", "#printer_fr_collapse"));
			ADD_PRINTER_ERROR(PrinterFRNotAvailable, QCoreApplication::translate("PrinterStatuses", "#printer_fr_not_available"));
			ADD_PRINTER_ERROR(Motor,                 QCoreApplication::translate("PrinterStatuses", "#motor_error"));
			ADD_PRINTER_ERROR(NoToner,               QCoreApplication::translate("PrinterStatuses", "#no_toner"));
			ADD_PRINTER_ERROR(OutletFull,            QCoreApplication::translate("PrinterStatuses", "#outlet_full"));
			ADD_PRINTER_ERROR(NeedPaperTakeOut,      QCoreApplication::translate("PrinterStatuses", "#need_paper_take_out"));
			ADD_PRINTER_ERROR(MemoryEnd,             QCoreApplication::translate("PrinterStatuses", "#memory_end"));
		}

		TStatusCodes getAvailableErrors()
		{
			using namespace DeviceStatusCode;

			return mStatusCodes - (TStatusCodes()
				<< OK::PaperInPresenter
				<< OK::MotorMotion
				<< Error::PrinterFRNotAvailable
				<< DeviceStatusCode::Error::MechanismPosition
				<< DeviceStatusCode::Error::Temperature
				<< DeviceStatusCode::Error::CoverIsOpened
				<< DeviceStatusCode::Error::MechanicFailure);
		}

	private:
		TStatusCodes mStatusCodes;
	};
}

//--------------------------------------------------------------------------------
