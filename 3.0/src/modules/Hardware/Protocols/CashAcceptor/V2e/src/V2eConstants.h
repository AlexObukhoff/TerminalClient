/* @file Константы протокола V2e. */

#pragma once

#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
namespace CV2e
{
	/// Первый байт при упаковке данных.
	const char Prefix = ASCII::STX;

	/// Байт версии протокола.
	const char ProtocolMark = ASCII::NUL;

	/// Минимальный размер ответа.
	const int MinAnswerSize = 7;

	/// Таймаут для чтения ответа по умолчанию, [мс].
	const int AnswerTimeout = 300;

	/// ACK.
	const char ACK = ASCII::NUL;

	/// NAK.
	const char NAK = ASCII::Full;

	/// IRQ.
	const char IRQ = '\x55';

	/// Количество максимальных повторов.
	const int MaxRepeat = 3;

	/// Переспросить ответ.
	const char Retransmit = '\x77';
}
//--------------------------------------------------------------------------------
