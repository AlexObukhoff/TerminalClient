/* @file Ответ сервера на запрос статуса платежа. */

// Project
#include "PaymentStatusResponse.h"

//---------------------------------------------------------------------------
PaymentStatusResponse::PaymentStatusResponse(const Request & aRequest, const QString & aResponseString)
	: PaymentResponse(aRequest, aResponseString)
{
}

//---------------------------------------------------------------------------
bool PaymentStatusResponse::isOk()
{
	return ((getError() == EServerError::Ok) && (getResult() == EServerResult::StatusCheckOk)) ? true : false;
}

//---------------------------------------------------------------------------
