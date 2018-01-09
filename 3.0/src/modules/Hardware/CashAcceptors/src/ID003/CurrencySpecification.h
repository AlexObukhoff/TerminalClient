/* @file Спецификация валют протокола ID003. */

#pragma once

// SDK
#include <SDK/Drivers/CashAcceptor/CurrencyList.h>

// Project
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CID003
{
	class CCurrencyCodes : public CSpecification<char, int>
	{
	public:
		CCurrencyCodes()
		{
			using namespace Currency;

			append('\x27', RUB);
			append('\xE0', EUR);
			append('\x01', USD);
			append('\x08', CAD);
			append('\x5C', UAH);
			append('\x51', KZT);
			append('\x86', UZS);
			append('\x7E', MDL);
			append('\x30', HUF);
			append('\x16', CHF);
			append('\x2D', CNY);
			append('\x63', INR);
			append('\x78', IRR);

			setDefault(Currency::NoCurrency);
		}
	};

	static CCurrencyCodes CurrencyCodes;
}

//--------------------------------------------------------------------------------
