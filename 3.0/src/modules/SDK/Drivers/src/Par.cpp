/* @file Описатель валюты для устройств приема денег. */

#pragma once

#include <SDK/Drivers/CashAcceptor/CurrencyList.h>
#include "Par.h"

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
SPar::SPar(): nominal(0), currencyId(Currency::NoCurrency), enabled(true), inhibit(true), cashReceiver(ECashReceiver::BillAcceptor)
{
}

//--------------------------------------------------------------------------------
SPar::SPar(double aNominal, const QString & aCurrency, ECashReceiver::Enum aCashReceiver, bool aEnabled, bool aInhibit) :
	nominal(aNominal),
	currencyId(Currency::NoCurrency),
	currency(aCurrency),
	enabled(aEnabled),
	inhibit(aInhibit),
	cashReceiver(aCashReceiver)
{
	inhibit = aInhibit || !aNominal;
}

//--------------------------------------------------------------------------------
SPar::SPar(double aNominal, int aCurrencyId, ECashReceiver::Enum aCashReceiver, bool aEnabled, bool aInhibit) :
	nominal(aNominal),
	currencyId(aCurrencyId),
	enabled(aEnabled),
	inhibit(aInhibit),
	cashReceiver(aCashReceiver)
{
	inhibit = aInhibit || !aNominal;
}

//--------------------------------------------------------------------------------
bool SPar::operator==(const SPar & aPar) const
{
	return qFuzzyCompare(nominal, aPar.nominal) &&
		(cashReceiver == aPar.cashReceiver) &&
		((currencyId == aPar.currencyId) ||
		((currencyId == Currency::RUB) && (aPar.currencyId == Currency::RUR)) ||
		((currencyId == Currency::RUR) && (aPar.currencyId == Currency::RUB)));
}

//--------------------------------------------------------------------------------
bool SPar::isEqual(const SPar & aPar) const
{
	return (*this == aPar) && (enabled == aPar.enabled);
}

//--------------------------------------------------------------------------------
bool SPar::operator < (const SPar & aPar) const
{
	if (!qFuzzyCompare(nominal, aPar.nominal))
	{
		return (nominal < aPar.nominal);
	}

	if (currencyId != aPar.currencyId)
	{
		return (currencyId < aPar.currencyId);
	}

	return (cashReceiver < aPar.cashReceiver);
}

}} // namespace SDK::Driver

bool isParListEqual(SDK::Driver::TParList aParList1, SDK::Driver::TParList aParList2)
{
	if (aParList1.size() != aParList2.size())
	{
		return false;
	}

	qSort(aParList1);
	qSort(aParList2);

	for (int i = 0; i < aParList1.size(); ++i)
	{
		if (!aParList1[i].isEqual(aParList2[i]))
		{
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
