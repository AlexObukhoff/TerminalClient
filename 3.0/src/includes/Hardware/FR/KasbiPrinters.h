/* @file Модели принтеров для Касби. */

#pragma once

#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CKasbiPrinters
{
	const char Auto    = '\x00';    /// Автоопределение.
	const char Virtual = '\xFE';    /// Печать в никуда.

	const char Default[] = "Auto";    /// По умолчанию.

	class CModels: public CDescription<char>
	{
	public:
		CModels()
		{
			append(0, "Auto");
			append(1, "Custom VKP-80");
			append(2, "Custom TG-2480");
			append(3, "Citizen CT-S2000");
			append(4, "Citizen PPU-700");
			append(5, "Star TSP");
			append(6, "Star TUP");
			append(7, "Epson EU-422");
			append(Virtual, "Virtual");
		}

		QStringList getNames()
		{
			QStringList result = data().values();
			result.removeAll(value(Auto));
			result.removeAll(value(Virtual));

			return result;
		}
	};

	static CModels Models;
}

//--------------------------------------------------------------------------------
