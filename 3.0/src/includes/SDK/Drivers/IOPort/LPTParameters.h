/* @file Параметры LPT-порта. */

#pragma once

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
/// Адреса регистров.
namespace CLPTPortAddress
{
	const int DataRegister    = 0x378;
	const int StatusRegister  = 0x379;
	const int ControlRegister = 0x37A;
}

//--------------------------------------------------------------------------------
/// Параметры LPT-порта.
namespace ELPTPortParameters
{
	enum Enum
	{
		int portNumber; /// Номер порта.
	};
}

}} // namespace SDK::Driver

//--------------------------------------------------------------------------------

