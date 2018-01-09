/* @file Константы принтеров Fujitsu на контроллере Trentino FTP-609. */

// Project
#include "Hardware/Printers/PrinterStatusCodes.h"
#include "Hardware/Printers/Tags.h"

using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
namespace CFujitsu
{
	/// Статусы.
	class CStatuses : public CSpecification<int, int>
	{
	public:
		CStatuses()
		{
			append(0, PrinterStatusCode::Warning::PaperNearEnd);
			append(1, PrinterStatusCode::Error::PaperEnd);
			append(2, PrinterStatusCode::Error::Temperature);
			append(3, PrinterStatusCode::Error::PrintingHead);
			append(4, PrinterStatusCode::Error::Cutter);
			append(5, PrinterStatusCode::Error::PaperJam);
			append(7, DeviceStatusCode::Warning::OperationError);    //TODO: buffer full
			append(7, DeviceStatusCode::Error::NotAvailable);
		}
	};

	static CStatuses Statuses;

	/// Напряжение.
	namespace Voltage
	{
		const int Nominal  = 24;
		const double Delta = 2.4;  //V = Vnom +/- Vdelta
	}

	/// Команды.
	namespace Commands
	{
		const char Identification[] = "\x17";
		const char Initialize[] = "\x16";
		const char Status[] = "\x18";
		const char Voltage[] = "\x19";
	}

	//----------------------------------------------------------------------------
	/// Теги.
	class TagEngine : public Tags::Engine
	{
	public:
		TagEngine()
		{
			appendSingle(Tags::Type::UnderLine,    "", "\x11", "\x10");
			appendSingle(Tags::Type::DoubleWidth,  "", "\x04", "\x03");
			appendSingle(Tags::Type::DoubleHeight, "", "\x05", "\x03");
		}
	};
};

//--------------------------------------------------------------------------------
