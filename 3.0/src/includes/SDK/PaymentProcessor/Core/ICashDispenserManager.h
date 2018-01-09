/* @file Интерфейс обеспечивающий взаимодействие с системой выдачи средств. */

#pragma once

// Stl
#include <tuple>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QVector>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Payment/Amount.h>

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
struct SCashUnit
{
	QString currencyName;
	int nominal;
	int count;

	SCashUnit() : nominal(0), count(0) {}
	SCashUnit(const QString & aCurrencyName, int aNominal = 0, int aCount = 0) : currencyName(aCurrencyName), nominal(aNominal), count(aCount) {}

	bool operator==(const SCashUnit & aUnit) const
	{
		return currencyName == aUnit.currencyName && nominal == aUnit.nominal && count == aUnit.count;
	}

	int amount() const { return nominal * count; }
};

//------------------------------------------------------------------------------
typedef QVector<SCashUnit> TCashUnitList;
typedef TCashUnitList::iterator TCashUnitListIt;
typedef QMap<QString, TCashUnitList> TCashUnitsState;

//------------------------------------------------------------------------------
class ICashDispenserManager : public QObject
{
	Q_OBJECT

public:
	/// Проверка, возможна ли выдача наличных средств
	virtual SDK::PaymentProcessor::TPaymentAmount canDispense(TPaymentAmount aRequiredAmount) = 0;

	/// Выдать указанную сумму (асинхронная операция)
	virtual void dispense(TPaymentAmount aAmount) = 0;

	/// Получить список номиналов для всех кассетниц всех устройств и захватить мир
	virtual TCashUnitsState getCashUnitsState() = 0;

	/// Установить список номиналов для всех кассетниц
	virtual bool setCashUnitsState(const QString & aDeviceConfigurationName, const TCashUnitList & aCashUnitList) = 0;

signals:
	/// Выдана указанная сумма
	void dispensed(double aAmount);

	/// Сигнал об активности. Пример: купюра втянута в ящик сброса.
	void activity();

	/// Сигнал срабатывает при ошибке выдачи средств. В aError находится нелокализованныя ошибка.
	void error(QString aError);

protected:
	virtual ~ICashDispenserManager() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor

Q_DECLARE_METATYPE(SDK::PaymentProcessor::SCashUnit);

