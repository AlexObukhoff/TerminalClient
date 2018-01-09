/* @file Константы купюроприемников Creator на протоколе CCNet. */

#pragma once

//--------------------------------------------------------------------------------
namespace CCCNetCreator
{
	/// Формат представления даты в ответе на запросы идентификации.
	const QString DateFormat = "yyyyMM";

	/// Формат представления даты для формирования данных устройства.
	const QString DateLogFormat = "MM.yyyy";

	/// Выход из initilaize-а.
	const int ExitInitializeTimeout = 5 * 1000;

	/// Команды
	namespace Commands
	{
		/// Обновление прошивки. A1..A3 - это вроде как команды купюрника
		namespace UpdatingFirmware
		{
			const char SetBaudRate[] = "\xA0";
			const char WriteHead[]   = "\xA1";
			const char WriteBlock[]  = "\xA2";
			const char Exit[]        = "\xA3";
		}

		const char GetInternalVersion[] = "\x70";
		const char GetSerial[] = "\x72";
	}

	/// Обновление прошивки.
	namespace UpdatingFirmware
	{
		/// Размер заголовка прошивки.
		const int HeadSize = 39;

		/// Размер блока прошивки.
		const int BlockSize = 128;

		/// Ответы на команды обновления прошивки.
		namespace Answers
		{
			const char WritingBlockOK = '\xA4';
			const char WritingBlockError = '\xA5';
		}
	}
}

//--------------------------------------------------------------------------------
