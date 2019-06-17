/* @file Константы протокола ФР АТОЛ3. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
namespace CAtol3FR
{
	typedef QPair<QByteArray, QByteArray> TReplaceableData;
	typedef QList<TReplaceableData> TReplaceableDataList; 

	const TReplaceableDataList ReplaceableDataList = TReplaceableDataList()
		<< TReplaceableData("\xFE", "\xFD\xEE")
		<< TReplaceableData("\xFD", "\xFD\xED");

	/// Префикс.
	const char Prefix = '\xFE';

	/// Маска длины.
	const char SizeMask = '\x7F';

	/// Последний Id.
	const char LastId = '\xDF';

	/// Асинхронный Id.
	const char AsyncId = '\xF0';

	/// Пароль.
	const QByteArray Password = QByteArray(2, ASCII::NUL);

	/// Минимальный размер транспортной части ответа.
	const int MinAnswerSize = 5;

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

	/// Команды.
	namespace Commands
	{
		const char Add    = '\xC1';    /// Добавить задание в буфер.
		const char ACK    = '\xC2';    /// Подтвердить получение результата.
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
		/// На ACK.
		const int ACK = 500;

		/// На очистку буфера.
		const int Cancel = 100;

		/// На запрос результата.
		const int GetResult = 500;
	}
}

//--------------------------------------------------------------------------------
