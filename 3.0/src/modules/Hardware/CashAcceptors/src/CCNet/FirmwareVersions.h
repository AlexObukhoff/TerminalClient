/* @file Данные о последних версиях прошивок. */

#pragma once

// SDK
#include <SDK/Drivers/CashAcceptor/CurrencyList.h>

// Modules
#include "Hardware/Common/Specifications.h"

// Project
#include "Models.h"

//--------------------------------------------------------------------------------
namespace CCCNet
{
	typedef QSet<int> TFimwareVersionSet;
	typedef QMap<int, QMap<bool, TFimwareVersionSet> > TFimwareVersions;

	class CFimwareVersions : public CSpecification<QString, TFimwareVersions>
	{
	public:
		CFimwareVersions()
		{
			data()[Models::CashcodeGX ][Currency::RUB][true]  = TFimwareVersionSet() << 1205;

			data()[Models::CashcodeSM ][Currency::RUB][true]  = TFimwareVersionSet() << 1357;

			data()[Models::CashcodeSM ][Currency::RUB][false] = TFimwareVersionSet() << 1386 << 1434;
			data()[Models::CashcodeMSM][Currency::RUB][false] = TFimwareVersionSet() << 1115;
			data()[Models::CashcodeMSM][Currency::EUR][false] = TFimwareVersionSet() << 1130 << 1228 << 1329 << 1411 << 1527;
			data()[Models::CashcodeMVU][Currency::RUB][false] = TFimwareVersionSet() << 1330;
			data()[Models::CashcodeMFL][Currency::RUB][false] = TFimwareVersionSet() << 1143;
			data()[Models::CashcodeSL ][Currency::RUB][false] = TFimwareVersionSet() << 1013 << 2004 << 3003 << 0005;

			data()[Models::CashcodeMVU][Currency::KZT][false] = TFimwareVersionSet() << 1314;
			data()[Models::CashcodeMFL][Currency::KZT][false] = TFimwareVersionSet() << 1124;
			data()[Models::CashcodeMSM][Currency::KZT][false] = TFimwareVersionSet() << 1126;

			data()[Models::CashcodeG200][Currency::RUB][false] = TFimwareVersionSet() << 1523;
		}
	};

	class COutdatedFimwareSeries : public CSpecification<QString, TFimwareVersions>
	{
	public:
		COutdatedFimwareSeries()
		{
			data()[Models::CashcodeSM ][Currency::RUB][false] = TFimwareVersionSet() << 1600 << 1500;
			data()[Models::CashcodeMVU][Currency::KZT][false] = TFimwareVersionSet() << 1700;
		}
	};

	static CFimwareVersions FimwareVersions;
	static COutdatedFimwareSeries OutdatedFimwareSeries;
}

//--------------------------------------------------------------------------------
