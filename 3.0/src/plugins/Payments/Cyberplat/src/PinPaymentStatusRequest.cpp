/* @file Реализация запроса статуса для пинового платежа. */

// Project
#include "PinPaymentStatusRequest.h"

//---------------------------------------------------------------------------
PinPaymentStatusRequest::PinPaymentStatusRequest(Payment * aPayment)
	: PaymentStatusRequest(aPayment)
{
	addParameter("PIN_DATA", 1);
}

//---------------------------------------------------------------------------
