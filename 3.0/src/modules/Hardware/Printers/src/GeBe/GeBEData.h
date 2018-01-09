/* @file Константы принтеров GeBE. */

#pragma once

#include "Hardware/Printers/Tags.h"
#include "Hardware/Printers/PrinterStatusCodes.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний принтеров GeBE.
namespace CGeBE
{
	/// Max число символов, передаваемых на печать за раз.
	const int LineSize = 28;

	/// Статусы.
	class CStatuses : public CSpecification<char, int>
	{
	public:
		CStatuses()
		{
			append('H', PrinterStatusCode::Error::PrintingHead);
			append('P', PrinterStatusCode::Error::PaperEnd);
			append('Z', PrinterStatusCode::Warning::PaperNearEnd);
			append('G', PrinterStatusCode::Warning::PaperNearEnd);
			append('C', PrinterStatusCode::Error::Cutter);
			append('K', PrinterStatusCode::Error::Temperature);
			append('I', PrinterStatusCode::Error::Temperature);
			append('U', DeviceStatusCode::Error::PowerSupply);
			append('M', DeviceStatusCode::Error::PowerSupply);
			append('?', PrinterStatusCode::Error::Port);
			append('E', DeviceStatusCode::Error::MemoryStorage);

			setDefault(DeviceStatusCode::OK::OK);
		}
	};

	static CStatuses Statuses;

	/// Команды.
	namespace Commands
	{
		const char Initilize[]    = "\x1B\x40";            /// Инициализация.
		const char SetFont[]      = "\x1B\x50\x31";        /// Установить шрифт.
		const char SetLeftAlign[] = "\x1B\x4E\x01\xA0";    /// Установить выравнивание слева.
		const char GetStatus[]    = "\x1B\x6B\xFF";        /// Cтатус.
	}

	/// Теги.
	class TagEngine : public Tags::Engine
	{
	public:
		TagEngine()
		{
			appendSingle(Tags::Type::UnderLine,    "\x1B\x4C", "\x31", "\x30");
			appendSingle(Tags::Type::DoubleWidth,  "\x1B\x57", "\x31", "\x30");
			appendSingle(Tags::Type::DoubleHeight, "\x1B\x48", "\x31", "\x30");
		}
	};
}

//--------------------------------------------------------------------------------
