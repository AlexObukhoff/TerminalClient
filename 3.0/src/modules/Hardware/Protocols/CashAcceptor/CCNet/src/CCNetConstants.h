/* @file Константы протокола CCNet. */

#pragma once

#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
namespace CCCNet
{
	/// Первый байт при упаковке данных.
	const char Prefix = ASCII::STX;

	/// ACK.
	const char ACK = ASCII::NUL;

	/// NAK.
	const char NAK = ASCII::Full;

	/// Константа для подсчета контрольной суммы.
	const ushort Polynominal = 0x8408;

	/// Минимальный размер отклика от валидатора.
	const int MinAnswerSize  = 6;

	/// Максимальное количество повторов команды в случае NAK-а или незаконченного/неверного ответа.
	const int MaxRepeatPacket = 3;

	/// Пауза на смене скорости порта, [мс].
	const int ChangingBaudRatePause = 300;
}

//--------------------------------------------------------------------------------
