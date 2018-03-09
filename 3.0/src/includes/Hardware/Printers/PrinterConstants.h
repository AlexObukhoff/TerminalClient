/* @file Общие константы принтеров. */

#pragma once

#include "Common/QtHeadersBegin.h"
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include "Common/QtHeadersEnd.h"

//--------------------------------------------------------------------------------
namespace CPrinters
{
	/// Число пробелов в табуляции.
	const int SpacesInTAB = 8;

	/// Таймаут повтора неудавшейся ретракции, [с].
	const int ClearingPresenterRepeatTimeout = 90;

	/// Запас времени перед переполнением буфера (виртуальное переполнение), [мин].
	const int ZBufferVirtualOverflow = 15;

	/// Новая строка.
	const char LineSpacer[] = "\r\n";

	/// Количество символов в строке-разделителе по умолчанию.
	const int DefaultHRSize = 35;

	/// Действие с незабранным чеком.
	namespace ELeftReceiptAction
	{
		enum Enum
		{
			Default,    /// Ничего не делать, принтер сам разберется.
			Push,       /// Вытолкнуть.
			Retract     /// Забрать в ретрактор.
		};
	}
}

//--------------------------------------------------------------------------------
