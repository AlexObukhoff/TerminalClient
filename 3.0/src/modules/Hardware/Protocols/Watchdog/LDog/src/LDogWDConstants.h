/* @file Константы протокола сторожевого таймера LDog. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
namespace CLDogWD
{
	/// Адрес платы.
	const char Address = '\x10';

	/// Конечный байт пакета.
	const char Postfix = '\x0D';

	/// Маскированный конечный байт пакета.
	const char MaskedPostfix[] = "\x40\xCD";

	/// Минимальный размер ответного пакета.
	const int MinAnswerSize = 4;

	/// Маска корректного ответа для адреса и команды.
	const char AnswerMask = '\x80';

	typedef QPair<char, QByteArray> TReplaceData;
	typedef QList<TReplaceData> TReplaceDataList;

	/// Список данных для маскирования.
	const TReplaceDataList ReplaceDataList = TReplaceDataList()
		<< TReplaceData('\x40', QByteArray::fromRawData("\x40\x00", 2))
		<< TReplaceData('\x0D', MaskedPostfix);

	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// Чтение ответа.
		const ushort Answer = 2000;
	}
}

//--------------------------------------------------------------------------------
