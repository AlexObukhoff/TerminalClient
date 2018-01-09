/* @file Прокси класс для работы с купюроприёмниками и другими средствами приёма денег. */

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IFundsService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Scripting/FundsService.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
FundsService::FundsService(ICore * aCore)
	: mCore(aCore),
	  mFundsService(mCore->getFundsService()),
	  mAvaliableAmount(0.0)
{
	connect(mFundsService->getAcceptor(), SIGNAL(error(qint64, QString)), SIGNAL(error(qint64, QString)));
	connect(mFundsService->getAcceptor(), SIGNAL(warning(qint64, QString)), SIGNAL(warning(qint64, QString)));
	connect(mFundsService->getAcceptor(), SIGNAL(cheated(qint64)), SLOT(onCheated(qint64)));
	connect(mFundsService->getAcceptor(), SIGNAL(activity()), SIGNAL(activity()));
	connect(mFundsService->getAcceptor(), SIGNAL(disabled(qint64)), SIGNAL(disabled(qint64)));

	connect(mFundsService->getDispenser(), SIGNAL(error(QString)), SIGNAL(error2(QString)));
	connect(mFundsService->getDispenser(), SIGNAL(activity()), SIGNAL(activity2()));
	connect(mFundsService->getDispenser(), SIGNAL(dispensed(double)), SIGNAL(dispensed(double)));

	mCore->getEventService()->subscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event)));
}

//------------------------------------------------------------------------------
bool FundsService::enable(qint64 aPayment)
{
	return enable(aPayment, "", 0.0);
}

//------------------------------------------------------------------------------
bool FundsService::enable(qint64 aPayment, const QString & aPaymentMethod, QVariant aLimit)
{
	return mFundsService->getAcceptor()->enable(aPayment, aPaymentMethod, aLimit.toDouble());
}

//------------------------------------------------------------------------------
bool FundsService::disable(qint64 aPayment)
{
	return mFundsService->getAcceptor()->disable(aPayment);
}

//------------------------------------------------------------------------------
QStringList FundsService::getPaymentMethods() const
{
	return mFundsService->getAcceptor()->getPaymentMethods();
}

//------------------------------------------------------------------------------
bool FundsService::canDispense()
{
	TPaymentAmount aRequiredAmount = mCore->getPaymentService()->getChangeAmount();
	mAvaliableAmount = mFundsService->getDispenser()->canDispense(aRequiredAmount);

	return !qFuzzyIsNull(mAvaliableAmount);
}

//------------------------------------------------------------------------------
void FundsService::dispense()
{
	if (!qFuzzyIsNull(mAvaliableAmount))
	{
		mFundsService->getDispenser()->dispense(mAvaliableAmount);
		mAvaliableAmount = 0;
	}
}

//------------------------------------------------------------------------------
void FundsService::onCheated(qint64 aPayment)
{
	if (aPayment > 0)
	{
		mCore->getPaymentService()->updatePaymentField(aPayment,
			IPayment::SParameter(SDK::PaymentProcessor::CPayment::Parameters::Cheated,
			SDK::PaymentProcessor::EPaymentCheatedType::CashAcceptor, true, false, true));
	}
}

//------------------------------------------------------------------------------
void FundsService::onEvent(const SDK::PaymentProcessor::Event & aEvent)
{
	if (aEvent.getType() == SDK::PaymentProcessor::EEventType::Critical)
	{
		auto paymentId = mCore->getPaymentService()->getActivePayment();
		
		if (paymentId)
		{
			emit error(paymentId, aEvent.getData().toString());
		}
	}
}

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
