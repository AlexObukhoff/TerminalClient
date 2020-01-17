/* @file Класс для регистрации фискальных платежей. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Payment/Amount.h>
#include <SDK/Drivers/FR/FiscalDataTypes.h>

namespace SDK {
namespace PaymentProcessor {

namespace ERequestType
{
	enum Enum
	{
		Receipt = 1,
		XReport = 2,
		Encashment = 4,
		ZReport = 8,
		SessionData
	};
}

//------------------------------------------------------------------------------
class IFiscalRegister
{
public:
	/// Соединение открыто.
	static const char * OFDNotSentSignal; // SIGNAL(OFDNotSent(bool));

public:
	virtual ~IFiscalRegister() {}

	/// Соединяет сигнал данного класса со слотом приёмника.
	virtual bool subscribe(const char * aSignal, QObject * aReceiver, const char * aSlot) = 0;

	/// Отсоединяет сигнал данного класса от слота приёмника.
	virtual bool unsubscribe(const char * aSignal, QObject * aReceiver) = 0;

	/// Инициализация регистратора
	virtual bool initialize(const QMap<QString, QString> & aParameters) = 0;

	/// Возвращает список фискальных параметров платежа
	virtual QStringList getParameterNames() = 0;

	/// Получить список возможностей ФР
	virtual bool hasCapability(quint32 aCapabilityFlags) = 0;

	/// Проверить готовность к выполнению операции.
	virtual bool isReady(ERequestType::Enum aType) = 0;

	/// Зарегистрировать платёж и вернуть набор параметров
	virtual QVariantMap createFiscalTicket(qint64 aPaymentId, const QVariantMap & aPaymentParameters, const SDK::Driver::SPaymentData & aPaymentData, bool aWaitResult) = 0;

	/// Получить строки для чека с фискальной информацией, список параметров может модифицироваться!
	virtual QStringList getReceipt(qint64 paymentId, const QVariantMap & aPaymentParameters, const SDK::Driver::SPaymentData & aPaymentData) = 0;

	/// Выполнить сервисную операцию (инкассация, Z-отчет, X-отчет).
	virtual bool serviceRequest(ERequestType::Enum aType, QStringList & aReceipt, const QVariantMap & aParameters) = 0;
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
