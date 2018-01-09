/* @file Реализация запроса статуса платежа. */

#pragma once

// Project
#include "PaymentRequest.h"

//---------------------------------------------------------------------------
class PaymentStatusRequest : public PaymentRequest
{
public:
	PaymentStatusRequest(Payment * aPayment);
};

//---------------------------------------------------------------------------
