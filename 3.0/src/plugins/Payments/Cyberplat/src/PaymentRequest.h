/* @file Реализация платёжного запроса к серверу. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/CyberPlat/Request.h>

using namespace SDK::PaymentProcessor::CyberPlat;

class Payment;

//---------------------------------------------------------------------------
class PaymentRequest : public Request
{
public:
	PaymentRequest(Payment * aPayment, const QString & aName);

	/// Добавляет в запрос дополнительные параметры из описания оператора для указанного шага.
	void addProviderParameters(const QString & aStep);

	/// Возвращает платёж, ассоциированный с запросом.
	virtual Payment * getPayment() const;

	/// Возвращает название запроса.
	virtual const QString & getName() const;

protected:
	/// Платёж, ассоциированный с запросом.
	Payment * mPayment;

	/// Название запроса.
	QString mName;
};

//---------------------------------------------------------------------------
