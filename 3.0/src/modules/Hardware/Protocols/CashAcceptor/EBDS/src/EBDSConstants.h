/* @file Константы протокола EBDS. */

#pragma once

#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
namespace CEBDS
{
	/// Первый байт.
	const char Prefix  = ASCII::STX;

	/// Конечный байт (не считая CRC).
	const char Postfix = ASCII::ETX;

	/// Маска для ACK.
	const char ACKMask = '\x0F';

	/// Таймаут для чтения ответа по умолчанию, [мс].
	const int AnswerTimeout = 300;
}

//--------------------------------------------------------------------------------
