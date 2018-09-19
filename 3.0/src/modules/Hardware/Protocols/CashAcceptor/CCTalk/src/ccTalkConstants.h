/* @file Константы протокола ccTalk. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
namespace CCCTalk
{
	const char NAK  = '\x05';
	const char BUSY = '\x06';

	/// Константа для вычисления контрольной суммы.
	const ushort Polynominal = 0x1021;

	/// Последний бит для вычисления CRC.
	const ushort LastBit = 0x8000;

	/// Минимальный размер ответного пакета.
	const int MinAnswerSize = 5;

	/// Максимальное количество повторов из-за BUSY устройства.
	const int MaxBusyNAKRepeats = 3;

	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// Повтор после BUSY или NAK-а.
		const int NAKBusy = 1000;

		/// Дефолтный для ожидания ответа.
		const int Reading = 500;
	}
}

//--------------------------------------------------------------------------------
