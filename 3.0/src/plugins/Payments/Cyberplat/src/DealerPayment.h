/* @file Дилерский платёж через процессинг Киберплат. */

#pragma once

// Project
#include "Payment.h"

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
};

//---------------------------------------------------------------------------
