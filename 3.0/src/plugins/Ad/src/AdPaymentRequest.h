/* @file Реализация платёжного запроса к серверу. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/CyberPlat/Request.h>

using namespace SDK::PaymentProcessor::CyberPlat;

class AdPayment;

//---------------------------------------------------------------------------
class AdPaymentRequest : public Request
{
public:
	AdPaymentRequest(AdPayment * aPayment, const QString & aName);

	/// Добавляет в запрос дополнительные параметры из описания оператора для указанного шага.
	void addProviderParameters(const QString & aStep);

	/// Возвращает платёж, ассоциированный с запросом.
	virtual AdPayment * getPayment() const;

	/// Возвращает название запроса.
	virtual const QString & getName() const;

#pragma region Request interface

	/// Возвращает содержимое запроса в пригодном для логирования виде.
	virtual QString toLogString() const;

#pragma endregion

protected:
	/// Платёж, ассоциированный с запросом.
	AdPayment * mPayment;

	/// Название запроса.
	QString mName;

private:
	/// Список полей, которые нельзя логировать.
	QStringList mCryptedFields;
};

//---------------------------------------------------------------------------
