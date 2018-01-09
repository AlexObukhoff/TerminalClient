/* @file Описатель валюты для устройств приема денег. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

// Project
#include "CurrencyList.h"

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
namespace ECashReceiver
{
	enum Enum
	{
		BillAcceptor,
		CoinAcceptor
	};
}

struct SPar
{
	double nominal;    /// Номинал.
	int currencyId;    /// Цифровой код валюты.
	QString currency;  /// Буквенный код валюты.
	bool enabled;      /// Доступность номинала со стороны платежной логики (то, что может запретить сервис).
	bool inhibit;      /// Доступность номинала со стороны логики драйвера (неизвестная или неподдерживаемая валюта, 0-й номинал и пр.).
	ECashReceiver::Enum cashReceiver;   /// Приемник денежных средств.

	/// Поля, значения которых актуально для конкретной банкноты.

	QString serialNumber; /// Серийный номер купюры.

	SPar(): nominal(0), currencyId(Currency::NoCurrency), enabled(true), inhibit(true), cashReceiver(ECashReceiver::BillAcceptor) {}
	SPar(double aNominal, const QString & aCurrency, ECashReceiver::Enum aCashReceiver = ECashReceiver::BillAcceptor, bool aEnabled = true, bool aInhibit = true):
		nominal(aNominal), currencyId(Currency::NoCurrency), currency(aCurrency), enabled(aEnabled), inhibit(aInhibit), cashReceiver(aCashReceiver)
	{
		inhibit = aInhibit || !aNominal;
	}

	SPar(double aNominal, int aCurrencyId, ECashReceiver::Enum aCashReceiver = ECashReceiver::BillAcceptor, bool aEnabled = true, bool aInhibit = false):
		nominal(aNominal), currencyId(aCurrencyId), enabled(aEnabled), inhibit(aInhibit), cashReceiver(aCashReceiver)
	{
		inhibit = aInhibit || !aNominal;
	}

	bool operator==(const SPar & aPar) const
	{
		return qFuzzyCompare(nominal, aPar.nominal) &&
		   (cashReceiver == aPar.cashReceiver) &&
		   ((currencyId == aPar.currencyId) ||
		   ((currencyId == Currency::RUB) && (aPar.currencyId == Currency::RUR)) ||
		   ((currencyId == Currency::RUR) && (aPar.currencyId == Currency::RUB)));
	}

	bool isEqual(const SPar & aPar) const
	{
		return (*this == aPar) && (enabled == aPar.enabled);
	}

	bool operator<(const SPar & aPar) const
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
};

/// Таблица номиналов.
typedef QList<SPar> TParList;
typedef QMap<int, SPar> TParTable;
typedef QPair<int, SDK::Driver::SPar> TParData;

}} // namespace SDK::Driver

Q_DECLARE_METATYPE(SDK::Driver::SPar);
Q_DECLARE_METATYPE(SDK::Driver::TParTable);
Q_DECLARE_METATYPE(SDK::Driver::TParList);

//--------------------------------------------------------------------------------

