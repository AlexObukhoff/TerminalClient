/* @file Интерфейс обеспечивающий взаимодействие с системой приёма средств. */

#pragma once

// Stl
#include <tuple>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Payment/Amount.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class ICashAcceptorManager : public QObject
{
	Q_OBJECT

public:
	/// Возвращает список доступных методов оплаты.
	virtual QStringList getPaymentMethods() = 0;

	/// Начать приём денег для указанного платежа.
	virtual bool enable(qint64 aPayment, const QString & aPaymentMethod, TPaymentAmount aMaxAmount) = 0;

	/// Завершить приём денег для указанного платежа. Возвращает false, если платеж еще обрабатывается.
	virtual bool disable(qint64 aPayment) = 0;

signals:
	/// Сигнал срабатывает, когда в платёж добавляется новая купюра.
	void amountUpdated(qint64 aPayment, double aTotalAmount, double aAmount);

	/// Сигнал срабатывает при ошибке приёма средств. В aError находится нелокализованныя ошибка.
	void warning(qint64 aPayment, QString aError);

	/// Сигнал срабатывает при ошибке приёма средств. В aError находится нелокализованныя ошибка.
	void error(qint64 aPayment, QString aError);

	/// Сигнал срабатывает при подозрении на манипуляции с устрйоством приема денег.
	void cheated(qint64 aPayment);

	/// Сигнал об активности сервиса. Пример: отбракована купюра.
	void activity();

	/// Сигнал о выключении купюроприемника на прием денег.
	void disabled(qint64 aPayment);

protected:
	virtual ~ICashAcceptorManager() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor

