/* @file Режимы печати. */

#pragma once

namespace SDK {
namespace Driver {

namespace EPrintingModes
{
	enum Enum
	{
		None = 0,
		Continuous,
		Glue
	};
};

}} // namespace SDK::Driver

namespace DSDK = SDK::Driver;

//---------------------------------------------------------------------------
