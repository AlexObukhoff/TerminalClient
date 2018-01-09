/* @file Реализация запроса на проверку номера. */

#pragma once

// Project
#include "PaymentRequest.h"

//---------------------------------------------------------------------------
class PaymentCheckRequest : public PaymentRequest
{
public:
	PaymentCheckRequest(Payment * aPayment, bool aFake);
};

//---------------------------------------------------------------------------
