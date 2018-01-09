/* @file Коды состояний модемов. */

#pragma once

#include "Hardware/Common/BaseStatus.h"

//--------------------------------------------------------------------------------
namespace ModemStatusCode
{
	/// Ошибки.
	namespace Error
	{
		const int SIMError  = 151;    /// Ошибка сим-карты.
		const int NoNetwork = 152;    /// Отсутствует GSM сигнал.
	}
}

//--------------------------------------------------------------------------------
