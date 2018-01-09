/* @file Дилерский платёж через процессинг Киберплат. */

// SDK
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/CyberPlat/ErrorCodes.h>

// Project
#include "DealerPayment.h"
#include "PaymentFactory.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
DealerPayment::DealerPayment(PaymentFactory * aFactory)
	: Payment(aFactory)
{
}

//------------------------------------------------------------------------------
bool DealerPayment::check(bool /*aFakeCheck*/)
{
	toLog(LogLevel::Normal, QString("Payment %1. (dealer) %2, operator: %3 (%4), session: %5, amount_all: %6, amount: %7.")
		.arg(getID())
		.arg(CPayment::Requests::Pay)
		.arg(mProviderSettings.id)
		.arg(mProviderSettings.name)
		.arg(getSession())
		.arg(getAmountAll())
		.arg(getAmount()));

	setStateOk();

	toLog(LogLevel::Normal, QString("Payment %1. (dealer) Checked.").arg(getID()));

	return true;
}

//------------------------------------------------------------------------------
bool DealerPayment::pay()
{
	toLog(LogLevel::Normal, QString("Payment %1. (dealer) Paid.").arg(getID()));

	setStateOk();

	setParameter(SParameter(PPSDK::CPayment::Parameters::NumberOfTries, 1, true));

	return true;
}

//------------------------------------------------------------------------------
bool DealerPayment::status()
{
	toLog(LogLevel::Normal, QString("Payment %1. Status retrieved.").arg(getID()));

	setStateOk();

	return true;
}

//------------------------------------------------------------------------------
void DealerPayment::setStateOk()
{
	setParameter(SParameter(PPSDK::CPayment::Parameters::ServerError, EServerError::Ok, true));
	setParameter(SParameter(PPSDK::CPayment::Parameters::ServerResult, EServerResult::Ok, true));
	setParameter(SParameter(PPSDK::CPayment::Parameters::ErrorMessage, "", true));
}

//---------------------------------------------------------------------------
