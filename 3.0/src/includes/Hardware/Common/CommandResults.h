/* @file Результаты выполнения команд. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

// Project
#include "CommandResultData.h"

//--------------------------------------------------------------------------------
namespace CommandResult
{
	// протокол
	const int OK        = 0;    /// OK.
	const int Port      = 1;    /// Не удалось послать посылку/получить ответ из-за ошибки порта.
	const int Transport = 2;    /// Не удалось открыть/закрыть сессию/получали NAK-и.
	const int Protocol  = 3;    /// Ответ некорректен по протоколу.
	const int Driver    = 4;    /// Ошибка драйвера.
	const int NoAnswer  = 5;    /// Нет ответа.
	const int Id        = 6;    /// Id пакета или команда.
	const int CRC       = 7;    /// CRC.

	// девайс
	const int Answer = 100;     /// Ответ логически некорректен.
	const int Device = 101;     /// Логическая ошибка устройства.

	typedef QSet<int> TResults;
	const TResults ProtocolErrors = TResults() << Port << Transport << Protocol << Driver << NoAnswer << Id << CRC;
	const TResults PresenceErrors = TResults() << OK << Id << CRC << Answer << Device;    // Устройство присутствует.
}

#define CORRECT(aResult) !CommandResult::ProtocolErrors.contains(aResult)

namespace EResult
{
	enum Enum { OK, Fail, Error };
}

//--------------------------------------------------------------------------------
