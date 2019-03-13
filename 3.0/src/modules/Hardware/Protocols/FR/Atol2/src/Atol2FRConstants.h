/* @file Константы протокола ФР АТОЛ. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
namespace CAtol2FR
{
	/// Начало пакета - фиксированный байт.
	const char Prefix = ASCII::STX;

	/// Конец пакета - фиксированный байт.
	const char Postfix = ASCII::ETX;

	/// Байт маскировки.
	const char MaskByte = ASCII::DLE;

	/// Экранирующий DLE.
	const char DLEMask[] = "\x10\x10";

	/// Экранирующий ETX.
	const char ETXMask[] = "\x10\x03";

	/// Минимальный размер ответного пакета.
	const int MinAnswerSize = 5;

	/// Максимальное число повторений пакета в случае ошибки.
	const int MaxRepeatPacket = 5;

	/// Максимальное число попыток обмена технологическими пакетами.
	const int MaxServiceRequests = 3;

	/// Пароль.
	const QByteArray Password = QByteArray(2, ASCII::NUL);

	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// Дефолтный для ожидания ответа на команду.
		const int Default = 3000;

		/// Таймаут после NAK-а при открытии сессии.
		const int NAKOpeningSession = 500;

		/// Таймаут ACK-а ожидания открытия сессии.
		const int OpeningSession = 500;
	}
}

//--------------------------------------------------------------------------------
