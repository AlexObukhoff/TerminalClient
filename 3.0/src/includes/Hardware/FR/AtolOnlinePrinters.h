/* @file Модели принтеров для онлайновых фискальников АТОЛа. */

#pragma once

#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CAtolOnlinePrinters
{
	const char Trade  = '\x01';    /// Принтер, на базе которого собран торговый ФР.
	const char Memory = '\x07';    /// Печать в никуда.

	const char Default[] = "Default";    /// По умолчанию.
	const char CitizenPPU700[] = "Citizen PPU-700";    /// Citizen PPU-700.

	class CModels: public CDescription<char>
	{
	public:
		CModels()
		{
			append(1, "ATOL");
			append(2, "Custom VKP-80");
			append(3, "Citizen PPU-700");
			append(4, "Citizen CT-S2000");
			append(5, "Custom TG-2480");
			append(6, "Epson");
			append(7, "Memory");
			append(8, "Custom VKP-80SX");
			append(9, "SNBC BT-080");
			append(10, "SNBC BK-T680");
		}

		QStringList getNames()
		{
			QStringList result = data().values();
			result.removeAll(value(Trade));
			result.removeAll(value(Memory));
			result.prepend(Default);

			return result;
		}
	};

	static CModels Models;
}

//--------------------------------------------------------------------------------
