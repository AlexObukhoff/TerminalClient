/* @file Ответ сервера на запрос статуса платежа. */

#pragma once

// Project
#include "PaymentResponse.h"

//---------------------------------------------------------------------------
class PaymentStatusResponse : public PaymentResponse
{
public:
	PaymentStatusResponse(const Request & aRequest, const QString & aResponseString);

	/// Response: Предикат истинен, если ответ сервера не содержит ошибок.
	virtual bool isOk();
};

//---------------------------------------------------------------------------
