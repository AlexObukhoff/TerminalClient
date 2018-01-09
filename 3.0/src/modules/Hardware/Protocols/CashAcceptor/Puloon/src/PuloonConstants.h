/* @file Константы протокола Puloon. */

#pragma once

#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний устройств на протоколе Puloon.
namespace CPuloon
{
	/// Начало передачи данных.
	const char CommandMark = ASCII::EOT;

	/// Начало передачи данных.
	const char AnswerMark = ASCII::SOH;

	/// Префикс.
	const char Prefix = ASCII::STX;

	/// Постфикс.
	const char Postfix = ASCII::ETX;

	/// Минимальный размер ответного пакета.
	const int MinPacketAnswerSize = 6;

	/// Максимальное число повторений пакета в случае ошибки.
	const int MaxRepeatPacket = 3;

	/// Дефолтный таймаут чтения ответа, [мс].
	const int DefaultAnswerTimeout = 200;
}

//--------------------------------------------------------------------------------
