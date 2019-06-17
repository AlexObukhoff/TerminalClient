/* @file Дилерский платёж через процессинг Киберплат. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Payment/IPaymentFactory.h>

// Project
#include "Payment.h"
#include "DealerLocalData.h"


//------------------------------------------------------------------------------
class DealerPayment : public Payment
{
public:
	DealerPayment(PaymentFactory * aFactory);

protected:
	/// Запрос на проведение платежа.
	virtual bool check(bool aFakeCheck);

	/// Транзакция.
	virtual bool pay();

	/// Запрос статуса платежа.
	virtual bool status();

protected:
	/// Выставляет коды ошибок сервера в OK
	void setStateOk();

	bool haveLocalData();

	QString getAddinfo(QMap<QString, QString> & aValues);

	QString getAddFields();

protected:
	PaymentFactory * mFactory;
	DealerLocalData mLocalData;
};

//---------------------------------------------------------------------------
