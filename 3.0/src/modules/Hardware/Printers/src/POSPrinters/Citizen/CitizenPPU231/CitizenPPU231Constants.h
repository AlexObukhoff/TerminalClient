/* @file Константы принтера Citizen PPU-231. */

#pragma once

// Modules
#include "Hardware/Common/WaitingData.h"

// Project
#include "Hardware/Printers/Tags.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний принтера Citizen PPU-231.
namespace CCitizenPPU231
{
	/// Максимальное количество символов в строке.
	const int LineSize = 48;

	/// Статусы.
	class CStatuses : public CSpecification<int, int>
	{
	public:
		CStatuses()
		{
			append(0, PrinterStatusCode::Warning::PaperNearEnd);
			append(2, PrinterStatusCode::Error::PaperEnd);
			append(4, DeviceStatusCode::Error::NotAvailable);
		}
	};

	static CStatuses Statuses;

	//--------------------------------------------------------------------------------
	/// Штрих-коды.
	namespace Barcode
	{
		const char CodeSystem128 = '\x07';    /// Система штрих-кода - CODE128.
		const char Code128Spec = 'B';         /// Cпецификация (уточнение, подвид) системы Code128.
		const char Postfix = ASCII::NUL;      /// Постфикс.
	}

	/// Ожидание прихода статуса бумаги, [мс].
	const SWaitingData PaperStatusWaiting = SWaitingData(100, 500, true);

	//----------------------------------------------------------------------------
	/// Теги.
	class TagEngine : public Tags::Engine
	{
	public:
		TagEngine()
		{
			QByteArray prefix("\x1B\x21");

			appendCommon(Tags::Type::Bold,         prefix, "\x08");
			appendCommon(Tags::Type::DoubleWidth,  prefix, "\x20");
			appendCommon(Tags::Type::DoubleHeight, prefix, "\x10");
			appendCommon(Tags::Type::UnderLine,    prefix, "\x80");

			set(Tags::Type::BarCode);
		}
	};
}

//--------------------------------------------------------------------------------
