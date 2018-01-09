/* @file Запрос получения полей для следующего шага multistage шлюза. */

// SDK
#include <SDK/PaymentProcessor/Payment/Parameters.h>

// Project
#include "MultistagePaymentGetStepRequest.h"
#include "MultistagePayment.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
MultistagePaymentGetStepRequest::MultistagePaymentGetStepRequest(Payment * aPayment) : 
	PaymentRequest(aPayment, CPayment::Requests::GetStep)
{
	clear();

	MultistagePayment * payment = dynamic_cast<MultistagePayment *>(aPayment);

	Q_ASSERT(payment);

	addParameter("SD", payment->getKeySettings().sd);
	addParameter("AP", payment->getKeySettings().ap);
	addParameter("OP", payment->getKeySettings().op);

	addParameter("SESSION", payment->getSession());
	addParameter("COMMENT", payment->getInitialSession());

	addParameter(CMultistage::Protocol::Step, payment->currentStep());

	auto addFields = [&](const PPSDK::TProviderFields & aFields)
	{
		foreach (auto field, aFields)
		{
			PPSDK::IPayment::SParameter fieldValue = payment->getParameter(field.id);
			if (fieldValue.value.isValid())
			{
				addParameter(field.id, fieldValue.value);
			}
		}
	};

	// заполняем поля от 0-го шага
	addFields(payment->getProviderSettings().fields);

	// заполняем поля от предыдущих шагов
	foreach (auto step, payment->getHistory())
	{
		addFields(payment->getFieldsForStep(step));
	}

	addFields(payment->getFieldsForStep(payment->currentStep()));
}

//---------------------------------------------------------------------------
