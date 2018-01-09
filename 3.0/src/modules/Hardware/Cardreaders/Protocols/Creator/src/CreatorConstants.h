/* @file Константы протокола Creator. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
namespace CCreator
{
	/// Префикс.
	const char Prefix = '\xF2';

	/// Постфикс.
	const char Postfix = ASCII::ETX;

	/// Количество данных в пакете для USB канала.
	const int USBDataSize = 64;

	/// Количество байтов в пакете для USB канала.
	const int USBPacketSize = USBDataSize + 1;

	/// Пустой USB-пакет.
	const QByteArray EmptyUSBPacket = QByteArray(USBPacketSize, ASCII::NUL);

	/// Количество байтов в буфере для чтения ответа (независимо от наличия ответа) из USB-канала.
	const int USBAnswerSize = 10 * USBPacketSize;

	/// Пустой USB-буфер.
	const QByteArray EmptyUSBAnswer = QByteArray(USBAnswerSize, ASCII::NUL);

	/// Постфикс - заменитель NUL-последовательности байтов в конце USB-запросов для логгирования.
	const char NULLogPostfix[] = " NUL";

	/// Минимальный размер ответного пакета.
	const int MinPacketAnswerSize = 7;

	/// Максимальное число повторений пакета в случае прихода NAK.
	const int MaxRepeatCommandNAK = 3;

	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// Дефолтный для ожидания ответа на команду.
		const int DefaultAnswer = 1000;

		/// Для ожидания ответа на серию пакетов.
		const int SeriesOfPackets = 2 * 1000;
	}

	/// Пауза после NAK-а при посылке команды, [мс].
	const int NAKAnswerPause = 100;
}

//--------------------------------------------------------------------------------
