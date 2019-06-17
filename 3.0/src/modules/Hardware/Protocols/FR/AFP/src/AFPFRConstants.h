/* @file Константы протокола AFP. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
namespace CAFPFR
{
	/// Префикс.
	const char Prefix = ASCII::STX;

	/// Постфикс.
	const char Postfix = ASCII::ETX;

	/// Пароль для связи.
	const char Password[]  = "PIRI";

	/// Максимальное значение Id пакета.
	const char MaxId = '\xF0';

	/// Минимальный размер ответного пакета.
	const int MinAnswerSize = 9;

	/// Байт-разделитель.
	const char Separator = '\x1C';
}

//--------------------------------------------------------------------------------
