/* @file Реализация запроса статуса для пинового платежа. */

#pragma once

// Project
#include "PaymentStatusRequest.h"

//---------------------------------------------------------------------------
class PinPaymentStatusRequest :	public PaymentStatusRequest
{
public:
	PinPaymentStatusRequest(Payment * aPayment);
};

//---------------------------------------------------------------------------
