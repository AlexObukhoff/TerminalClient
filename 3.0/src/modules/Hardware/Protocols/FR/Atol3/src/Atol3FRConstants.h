/* @file Константы протокола ФР АТОЛ3. */

#pragma once

#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
namespace CAtol3FR
{
	const char STX = '\xFE';    /// STX.
	const char ESC = '\xFD';    /// ESC.
	const char ESCTSTX[] = "\xFD\xEE";    /// Экранирующий STX.
	const char ESCTESC[] = "\xFD\xED";    /// Экранирующий ESC.

	/// Маска длины.
	const char SizeMask = '\x7F';

	/// Последний Id.
	const char LastId = '\xDF';

	/// Подсчет CRC.
	namespace CRC
	{
		/// Начальное значние.
		const char StartValue = '\xFF';

		/// Полином x8 + x5 + x4 + x0.
		const char Polynominal = '\x31';

		/// Последний бит.
		const char LastBit = '\x80';
	}

	namespace TaskFlags
	{
		const char NeedResult    = '\x01';    /// Передача результата задания.
		const char IgnoreError   = '\x02';    /// Работа с ошибками.
		const char WaitAsyncData = '\x04';    /// Ожидание выполнения задания.
	}

	/// Состояния.
	namespace States
	{
		const char Pending     = '\xA1';    /// Помещено в буфер, ждем.
		const char InProgress  = '\xA2';    /// Исполняется.
		const char Result      = '\xA3';    /// Исполнено, ошибок нет.
		const char Error       = '\xA4';    /// Исполнено, есть ошибка.
		const char Stopped     = '\xA5';    /// Было в Pending, когда возникла ошибка при исполнении предшествующих заданий.
		const char AsyncResult = '\xA6';    /// В асинхронном ответе - исполнено, ошибок нет.
		const char AsyncError  = '\xA7';    /// В асинхронном ответе - исполнено, есть ошибка.
		const char Waiting     = '\xA8';    /// Исполняется в фоновом режиме. ждем данные от внешнего устройства.
	}

	/// Команды.
	namespace Commands
	{
		const char Add    = '\xC1';    /// Добавить задание в буфер.
		const char Ack    = '\xC2';    /// Подтвердить получение результата.
		const char Req    = '\xC3';    /// Получить состояние задания.
		const char Cancel = '\xC4';    /// Очистить буфер.
		const char AckAdd = '\xC5';    /// Подтвердить получение результата + очистить буфер.
	}

	/// Ошибки.
	namespace Errors
	{
		const char Overflow      = '\xB1';    /// Недостаточно места в буфере.
		const char AlreadyExists = '\xB2';    /// Уже есть задание с таким TId.
		const char NotFound      = '\xB3';    /// Задания с таким TId нет в буфере.
		const char IllegalValue  = '\xB4';    /// Недопустимое значение параметра. PIdx = 0 (для Flags), PIdx = 1 (для TId).
	}

	/// Таймауты ожидания ответа, [мс].
	namespace Timeouts
	{
		/// На очистку буфера.
		const int Cancel = 500;

		/// На запрос результата.
		const int GetResult = 500;
	}
}

//--------------------------------------------------------------------------------
