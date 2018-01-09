/* @file Реализация запроса на проведение платежа. */

// Project
#include "Payment.h"
#include "PaymentPayRequest.h"

//---------------------------------------------------------------------------
PaymentPayRequest::PaymentPayRequest(Payment * aPayment)
	: PaymentRequest(aPayment, CPayment::Requests::Pay)
{
	// Дата и время (UTC) платежа в системе Контрагента.
	addParameter("DATE", getPayment()->getCompleteDate().toString("dd.MM.yyyy hh:mm:ss"));
	// Уникальный цифровой идентификатор платежа в системе Контрагента (внешний номер платежа), длина не более 32 символов.
	addParameter("RRN", getPayment()->getInitialSession());

	addProviderParameters(CPayment::Requests::Pay);
}

//---------------------------------------------------------------------------
