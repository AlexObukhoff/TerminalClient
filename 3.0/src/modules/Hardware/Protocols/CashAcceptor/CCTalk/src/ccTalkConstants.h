/* @file Константы протокола ccTalk. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <Common/QtHeadersEnd.h>

namespace CCCTalk
{
	/// Адреса устройств по умолчанию.
	namespace Address
	{
		const uchar Common  = 0;    /// Широкополосная команда.
		const uchar Host    = 1;    /// Хост.
		const uchar Default = 2;    /// Устройство по умолчанию.
	}

	const int MinAnswerSize     = 5;    /// Минимальный размер ответного пакета.
	const int MaxBusyNAKRepeats = 3;    /// Максимальное количество повторов из-за BUSY устройства.

	/// Таймауты, [мс].
	namespace Timeouts
	{
		const int NAKBusy = 1000;    /// Повтор после BUSY или NAK-а.
	}

	const char NAK  = '\x05';
	const char BUSY = '\x06';
}

//--------------------------------------------------------------------------------
