/* @file Константы протокола ФР ПРИМ. */

#pragma once

#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний устройств на протоколе PRIM.
namespace CPrimFR
{
	/// Начало пакета - фиксированный байт.
	const char Prefix = ASCII::STX;

	/// Конец пакета - фиксированный байт.
	const char Postfix = ASCII::ETX;

	/// Пароль для связи.
	const char Password[]  = "AERF";

	/// Байт-разделитель.
	const char Separator = '\x1C';

	/// Признак конца посылки.
	const char AnswerEndMark[] = "\x1C\x03";

	/// Количество повторов
	namespace RepeatingCount
	{
		/// Посылка некорректна по протоколу.
		const int Protocol = 3;

		/// Команда выполняется.
		const int CommandInProgress = 5;
	}

	/// Минимальный размер ответного пакета.
	const int MinAnswerSize = 11;

	/// Пауза при повторном запросе ответа, [мс]
	const int CommandInProgressPause = 1000;

	/// Дефолтный таймаут для чтения, [мс]
	const int DefaultTimeout =  1000;

	/// Начальное значение отличительного байта.
	const char DiffBeginning = '\x21';

	/// Конечное значение отличительного байта.
	const char DiffEnding = '\x7E';
}

//--------------------------------------------------------------------------------
