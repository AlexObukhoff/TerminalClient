/* @file Коды состояний диспенсеров. */

#pragma once

#include "Hardware/Common/BaseStatus.h"

//--------------------------------------------------------------------------------
namespace DispenserStatusCode
{
	/// Предупреждения.
	namespace Warning
	{
		const int Unit0NearEmpty    = 810;    /// Кассета 0 почти пуста.
		const int Unit1NearEmpty    = 811;    /// Кассета 1 почти пуста.
		const int Unit2NearEmpty    = 812;    /// Кассета 2 почти пуста.
		const int Unit3NearEmpty    = 813;    /// Кассета 3 почти пуста.
		const int AllUnitsNearEmpty = 814;    /// Все кассеты почти пусты.
		const int Unit0Empty        = 815;    /// Кассета 0 пуста.
		const int Unit1Empty        = 816;    /// Кассета 1 пуста.
		const int Unit2Empty        = 817;    /// Кассета 2 пуста.
		const int Unit3Empty        = 818;    /// Кассета 3 пуста.
	}
	
	/// Ошибки.
	namespace Error
	{
		const int AllUnitsEmpty   = 830;    /// Все кассеты пусты.
		const int Unit0Opened     = 831;    /// Кассета 0 открыта.
		const int Unit1Opened     = 832;    /// Кассета 1 открыта.
		const int Unit2Opened     = 833;    /// Кассета 2 открыта.
		const int Unit3Opened     = 834;    /// Кассета 3 открыта.
		const int RejectingOpened = 835;    /// Лоток незабранных купюр открыт.
		const int Jammed          = 836;    /// Замятие.
	}
}

//--------------------------------------------------------------------------------
