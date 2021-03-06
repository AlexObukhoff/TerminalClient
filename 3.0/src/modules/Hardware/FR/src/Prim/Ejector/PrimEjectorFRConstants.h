/* @file Константы базового ФР ПРИМ c эжектором. */

#pragma once

//--------------------------------------------------------------------------------
namespace CPrimEjectorFRActions
{
	const char Push[]            = "101";    /// Вытолкнуть документ, находящийся в презентере.
	const char Retract[]         = "001";    /// Забрать документ, находящийся в презентере.
	const char SetLoopDisabled[] = "010";    /// Переключиться в режим без презентации.
	const char SetLoopEnabled[]  = "000";    /// Переключиться в режим с презентаций.
	const char Reset[]           = "002";    /// Reset принтера.
}

//--------------------------------------------------------------------------------
