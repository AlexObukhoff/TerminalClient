/* @file Реализация запроса статуса для пинового платежа. */

// Project
#include "Payment.h"
#include "PaymentStatusRequest.h"

//---------------------------------------------------------------------------
PaymentStatusRequest::PaymentStatusRequest(Payment * aPayment)
	: PaymentRequest(aPayment, CPayment::Requests::Status)
{
	clear();

	addParameter("SD", aPayment->getKeySettings().sd);
	addParameter("AP", aPayment->getKeySettings().ap);
	addParameter("OP", aPayment->getKeySettings().op);

	addParameter("SESSION", aPayment->getSession());

	addProviderParameters(CPayment::Requests::Status);
}

//---------------------------------------------------------------------------
