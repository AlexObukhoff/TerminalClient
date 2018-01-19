/* @file Класс для регистрации фискальных платежей. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Payment/Amount.h>
#include <SDK/Drivers/FR/FiscalDataTypes.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class IFiscalRegister
{
public:
	enum
	{
		Receipt = 1,
		Balance = 2,
		Encashment = 4,
		ZReport = 8
	};

public:
	virtual ~IFiscalRegister() {}

	/// Инициализация регистратора
	virtual bool initialize(const QMap<QString, QString> & aParameters) = 0;

	/// Возвращает список фискальных параметров платежа
	virtual QStringList getParameterNames() = 0;

	/// Получить список возможностей ФР
	virtual bool haveCapability(quint32 aCapabilityFlags) = 0;

	/// Зарегистрировать платёж и вернуть набор параметров
	virtual QVariantMap createFiscalTicket(qint64 aPaymentId, const QVariantMap & aPaymentParameters, const SDK::Driver::SPaymentData & aPaymentData) = 0;

	/// Получить строки для чека с фискальной информацией, список параметров может модифицироваться!
	virtual QStringList getReceipt(qint64 paymentId, const QVariantMap & aPaymentParameters) = 0;

	/// Получить отчёт по балансу ФР
	virtual QStringList balance() = 0;

	/// Инкассировать и получить текст для печати на чеке
	virtual QStringList encashment() = 0;

	/// Получить Z отчёт
	virtual QStringList getZreport() = 0;
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor

