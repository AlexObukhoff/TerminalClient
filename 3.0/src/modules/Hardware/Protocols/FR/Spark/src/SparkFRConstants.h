/* @file Константы протокола ФР SPARK. */

#pragma once

#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
namespace CSparkFR
{
	/// Префикс.
	const char Prefix = ASCII::STX;

	/// Постфикс.
	const char Postfix = ASCII::ETX;

	/// Минимальный размер ответного пакета.
	const int MinPacketAnswerSize = 3;

	/// Максимальное число повторений пакета в случае ошибки.
	const int MaxRepeatPacket = 3;
}

//--------------------------------------------------------------------------------
