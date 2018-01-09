/* @file Описатели валют для валидаторов. */

#pragma once

// SDK
#include <SDK/Drivers/CashAcceptor/CurrencyList.h>

// Project
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace Currency
{
	const char NoCurrencyCode[] = "XXX";

	class Codes : public CSpecification<QString, int>
	{
	public:
		Codes(int aDefault = NoCurrency) : CSpecification(aDefault)
		{
			append("RUB", RUB);
			append("RUR", RUR);
			append("RUS", RUB);

			append("UAH", UAH);
			append("UKR", UAH);

			append("KZT", KZT);
			append("KAZ", KZT);

			append("EUR", EUR);
			append("USD", USD);
			append("CAD", CAD);
			append("UZS", UZS);
			append("MDL", MDL);
			append("HUF", HUF);
			append("CNY", CNY);

			append("CHF", CHF);
			append("CHE", CHF);

			append("INR", INR);
			append("IND", INR);
			append("IRR", IRR);
		}

		bool isAccorded(int aCurrency1, int aCurrency2)
		{
			return ((aCurrency1 != NoCurrency) && (aCurrency1 == aCurrency2)) ||
			       ((aCurrency1 == RUB) && (aCurrency2 == RUR)) ||
			       ((aCurrency1 == RUR) && (aCurrency2 == RUB));
		}

		bool isAccorded(const QString & aCode, int aCurrency)
		{
			return isAccorded(value(aCode), aCurrency);
		}
	};
}

static Currency::Codes CurrencyCodes(Currency::NoCurrency);

//--------------------------------------------------------------------------------
