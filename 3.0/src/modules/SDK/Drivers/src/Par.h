/* @file Описатель валюты для устройств приема денег. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

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
	double nominal;                      /// Номинал.
	int currencyId;                      /// Цифровой код валюты.
	QString currency;                    /// Буквенный код валюты.
	bool enabled;                        /// Доступность номинала со стороны платежной логики (то, что может запретить сервис).
	bool inhibit;                        /// Доступность номинала со стороны логики драйвера (неизвестная или неподдерживаемая валюта, 0-й номинал и пр.).
	QString serialNumber;                /// Серийный номер купюры (актуально для конкретной банкноты).
	ECashReceiver::Enum cashReceiver;    /// Приемник денежных средств.

	SPar();
	SPar(double aNominal, const QString & aCurrency, ECashReceiver::Enum aCashReceiver = ECashReceiver::BillAcceptor, bool aEnabled = true, bool aInhibit = true);
	SPar(double aNominal, int aCurrencyId, ECashReceiver::Enum aCashReceiver = ECashReceiver::BillAcceptor, bool aEnabled = true, bool aInhibit = false);

	bool operator==(const SPar & aPar) const;
	bool isEqual(const SPar & aPar) const;
	bool operator<(const SPar & aPar) const;
};

/// Таблица номиналов.
typedef QList<SPar> TParList;
typedef QMap<int, SPar> TParTable;
typedef QPair<int, SDK::Driver::SPar> TParData;

}} // namespace SDK::Driver

bool isParListEqual(SDK::Driver::TParList aParList1, SDK::Driver::TParList aParList2);

Q_DECLARE_METATYPE(SDK::Driver::SPar);
Q_DECLARE_METATYPE(SDK::Driver::TParTable);
Q_DECLARE_METATYPE(SDK::Driver::TParList);

//--------------------------------------------------------------------------------

