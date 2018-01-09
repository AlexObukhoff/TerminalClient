/* @file Константы, коды команд и ответов протокола ФР Штрих. */

#pragma once

#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний устройств на протоколе Shtrih.
namespace CShtrihFR
{
	/// Префикс.
	const char Prefix = ASCII::STX;

	/// Максимальный размер данных.
	const int MaxCommandSize = 252;

	/// Минимальный размер ответного пакета.
	const int MinPacketAnswerSize = 5;

	/// Максимальное число повторений пакета в случае ошибки.
	const int MaxRepeatPacket = 3;

	/// Максимальное число попыток обмена технологическими пакетами.
	const int MaxServiceRequests = 3;

	/// Мусор в TCP-канале.
	const char Trash = '\xFF';

	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// Дефолтный чтения ответа.
		const int DefaultAnswer = 3150;

		/// Для ответа на ENQ.
		const int ENQAnswer = 100;
	}
}

//--------------------------------------------------------------------------------
