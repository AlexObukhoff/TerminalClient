/* @file Типы сетевых соединений. */

#pragma once

// Qt headers
#include "Common/QtHeadersBegin.h"
#include <QtCore/QString>
#include "Common/QtHeadersEnd.h"


//--------------------------------------------------------------------------------
namespace EConnectionTypes
{
	enum Enum
	{ 
		Unknown,   /// Неизвестное.
		Unmanaged, /// Неуправляемое/локальная сеть.
		Dialup,    /// Модемное соединение.
		Vpn        /// VPN.
	};

	inline QString getConnectionTypeName(Enum aType)
	{
		switch(aType)
		{
			case Unmanaged:  return "Unmanaged"; /// Неуправляемое/локальная сеть.
			case Dialup:     return "Dialup";    /// Модемное соединение.
			case Vpn:        return "Vpn";       /// VPN.
			case Unknown:
			default:         return "Unknown";
		}
	}
}

//--------------------------------------------------------------------------------

