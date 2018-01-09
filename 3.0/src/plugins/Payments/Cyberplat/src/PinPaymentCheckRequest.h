/* @file Реализация проверочного платёжного запроса к серверу для пинового платежа. */

#pragma once

// Проект
#include "PaymentCheckRequest.h"

//---------------------------------------------------------------------------
class PinPaymentCheckRequest : public PaymentCheckRequest
{
public:
	PinPaymentCheckRequest(Payment * aPayment, bool aFake);
};

//---------------------------------------------------------------------------
