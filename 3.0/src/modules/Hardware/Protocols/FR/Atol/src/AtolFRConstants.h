/* @file Константы протокола ФР АТОЛ. */

#pragma once

#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
namespace CAtolFR
{
	/// Начало пакета - фиксированный байт.
	const char Prefix = ASCII::STX;

	/// Конец пакета - фиксированный байт.
	const char Postfix = ASCII::ETX;

	/// Байт маскировки.
	const char MaskByte = ASCII::DLE;

	/// Минимальный размер ответного пакета.
	const int MinPacketAnswerSize = 5;

	/// Максимальное число повторений пакета в случае ошибки.
	const int MaxRepeatPacket = 5;

	/// Максимальное число попыток обмена технологическими пакетами.
	const int MaxServiceRequests = 3;

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
