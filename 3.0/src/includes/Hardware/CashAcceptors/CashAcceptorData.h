/* @file Базовые константы устройств приема денег. */

#pragma once

//--------------------------------------------------------------------------------
namespace CCashAcceptor
{
	/// Таймауты.
	namespace Timeout
	{
		/// Включение/выключение на прием купюр.
		const int SetEnable = 1000;

		/// Достижения статуса Escrow.
		const int Escrow = 2 * 1000;

		/// Начало отработки команды Return.
		const int Return = 1 * 1000;

		/// Выход из initilaize-а.
		const int ExitInitialize = 10 * 1000;

		/// Таймаут начала инициализации после Reset.
		const int Initialize = 2000;

		/// Таймаут перед включением/выключением.
		const int StartSetEnable = 600;
	}

	/// Минимальный из возможных интервал поллинга, девайс отключен на прием денег.
	const int MinimumPollingInterval = 800;

	/// Максимальное количество попыток софтварного резета.
	const int MaxCommandAttempt = 3;
}

//--------------------------------------------------------------------------------
