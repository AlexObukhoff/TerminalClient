/* @file Общие константы принтеров. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QColor>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"

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

	/// Белый цвет.
	const QRgb White = QColor(Qt::transparent).rgb();

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

	/// Автозамена символов.
	class CAutoCorrection: public CSpecification<QChar, QChar>
	{
	public:
		CAutoCorrection::CAutoCorrection()
		{
			add("«", "\"");
			add("»", "\"");
			add("˝", "\"");
			add("˵", "\"");
			add("˶", "\"");
			add("″", "\"");
			add("“", "\"");
			add("”", "\"");
			add("і", "i");    // кириллическая буква, украинский и белорусский языки.
			add("І", "I");    // кириллическая буква, украинский и белорусский языки.
		}

	private:
		void add(const char * aKey, const char * aValue)
		{
			mBuffer.insert(QString::fromUtf8(aKey)[0], QString::fromUtf8(aValue)[0]);
		}
	};

	static CAutoCorrection AutoCorrection;
}

//--------------------------------------------------------------------------------
