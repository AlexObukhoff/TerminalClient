/* @file Спецификация статусов системного приннтера. */

// windows
#include <windows.h>

// Modules
#include "Hardware/Common/DeviceCodeSpecificationBase.h"
#include "Hardware/Printers/PrinterStatusCodes.h"

//--------------------------------------------------------------------------------
namespace CWindowsPrinter
{
	#define ADD_SYSTEM_CODE(aCodeType, aCodeName, aStatusCodeType, aStatusCodeName) \
		appendStatus(aCodeType##aCodeName, aStatusCodeType##StatusCode::aStatusCodeName, QString(#aCodeName).replace("_", " ").toLower());

	#define ADD_SYSTEM_STATUS(aCodeName, aStatusCodeType, aStatusCodeName) ADD_SYSTEM_CODE(PRINTER_STATUS, aCodeName, aStatusCodeType, aStatusCodeName)
	#define ADD_SYSTEM_JOB_STATUS(aCodeName, aStatusCodeType, aStatusCodeName) ADD_SYSTEM_CODE(JOB_STATUS, aCodeName, aStatusCodeType, aStatusCodeName)
	
	/// Статусы.
	class Statuses : public DeviceCodeSpecificationBase<ulong>
	{
	public:
		Statuses()
		{
			ADD_SYSTEM_STATUS(_BUSY, Device, OK::Busy);
			ADD_SYSTEM_STATUS(_DOOR_OPEN, Device, Error::CoverIsOpened);
			ADD_SYSTEM_STATUS(_ERROR, Device, Error::Unknown);
			ADD_SYSTEM_STATUS(_INITIALIZING, Device, OK::Initialization);
			ADD_SYSTEM_STATUS(_MANUAL_FEED, Device, OK::Busy);
			ADD_SYSTEM_STATUS(_NO_TONER, Printer, Error::NoToner);
			ADD_SYSTEM_STATUS(_NOT_AVAILABLE, Device, Error::NotAvailable);
			ADD_SYSTEM_STATUS(_OFFLINE, Device, Error::Unknown);
			ADD_SYSTEM_STATUS(_OUT_OF_MEMORY, Device, Error::MemoryStorage);
			ADD_SYSTEM_STATUS(_OUTPUT_BIN_FULL, Printer, Error::OutletFull);
			ADD_SYSTEM_STATUS(_PAGE_PUNT, Device, Warning::OperationError);
			ADD_SYSTEM_STATUS(_PAPER_JAM, Printer, Error::PaperJam);
			ADD_SYSTEM_STATUS(_PAPER_OUT, Printer, Error::PaperEnd);
			ADD_SYSTEM_STATUS(_PAPER_PROBLEM, Printer, Error::Paper);
			ADD_SYSTEM_STATUS(_PENDING_DELETION, Device, Warning::ThirdPartyDriver);
			ADD_SYSTEM_STATUS(_PRINTING, Device, OK::Busy);
			ADD_SYSTEM_STATUS(_PROCESSING, Device, OK::Busy);
			ADD_SYSTEM_STATUS(_SERVER_UNKNOWN, Device, OK::Unknown);
			ADD_SYSTEM_STATUS(_TONER_LOW, Printer, Warning::TonerNearEnd);
			ADD_SYSTEM_STATUS(_USER_INTERVENTION, Device, Error::Maintenance);
			ADD_SYSTEM_STATUS(_WARMING_UP, Device, OK::Busy);
			ADD_SYSTEM_STATUS(_DRIVER_UPDATE_NEEDED, Device, Warning::Firmware);
		}
	};

	/// Статусы заданий печати.
	class JobStatuses : public DeviceCodeSpecificationBase<ulong>
	{
	public:
		JobStatuses()
		{
			ADD_SYSTEM_JOB_STATUS(_BLOCKED_DEVQ, Device, Error::Unknown);
			ADD_SYSTEM_JOB_STATUS(_ERROR, Device, Error::Unknown);
			ADD_SYSTEM_JOB_STATUS(_OFFLINE, Device, Error::Unknown);
			ADD_SYSTEM_JOB_STATUS(_PAPEROUT, Printer, Error::PaperEnd);
			ADD_SYSTEM_JOB_STATUS(_USER_INTERVENTION, Device, Error::Maintenance);
		}
	};
}

static CWindowsPrinter::Statuses WindowsPrinterStatuses;
static CWindowsPrinter::JobStatuses WindowsPrinterJobStatuses;

//--------------------------------------------------------------------------------
