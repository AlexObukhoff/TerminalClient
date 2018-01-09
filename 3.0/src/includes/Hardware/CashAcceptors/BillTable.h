/* @file Описатель таблицы номиналов. */

#pragma once

// SDK
#include <SDK/Drivers/CashAcceptor/Par.h>

// Modules
#include "Hardware/Common/Specifications.h"
#include "Hardware/CashAcceptors/CurrencyDescriptions.h"

//--------------------------------------------------------------------------------
class CBillTable : public CSpecification<int, SDK::Driver::SPar>
{
public:
	void add(int aEscrow, const SDK::Driver::SPar & aPar)
	{
		SDK::Driver::SPar par(aPar);

		if (!CurrencyCodes.data().contains(par.currency) && CurrencyCodes.data().values().contains(par.currencyId))
		{
			par.currency = CurrencyCodes.key(par.currencyId);
		}

		append(aEscrow, par);
	}
};

//--------------------------------------------------------------------------------
