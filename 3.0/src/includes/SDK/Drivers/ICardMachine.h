/* @file Интерфейс банковского POS-терминала. */
#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Payment/Amount.h>

//------------------------------------------------------------------------------
namespace SDK {
namespace Driver {

class ICardMachine
{
public:
	/// Списание денег выполнено.
	static const char * SaleCompleteSignal; // = SIGNAL(saleComplete(double aAmount, int aCurrency, const QString & aRRN, const QString & aConfirmationCode, const QStringList& aReceipt))));

	/// Вывести сообщение в интерфейсе.
	static const char * MessageSignal; // = SIGNAL(message(const QString & aMessage));

	/// Начать процедуру оплаты
	virtual bool sale(SDK::PaymentProcessor::TPaymentAmount aAmount) = 0;

	/// Отсоединиться
	virtual void disable() = 0;

protected:
	virtual ~ICardMachine() {}
};

}} // namespace SDK::Driver


//--------------------------------------------------------------------------------
