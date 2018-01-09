/* @file Ответ сервера на получения полей для следующего шага multistage шлюза. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtXml/QDomDocument>
#include <Common/QtHeadersEnd.h>

// Project
#include "PaymentRequest.h"
#include "MultistagePaymentGetStepRequest.h"
#include "MultistagePaymentGetStepResponse.h"

using namespace SDK::PaymentProcessor::CyberPlat;

//---------------------------------------------------------------------------
MultistagePaymentGetStepResponse::MultistagePaymentGetStepResponse(const Request & aRequest, const QString & aResponseString)
	: PaymentResponse(aRequest, aResponseString)
{
	mIsOk = false;

	const PaymentRequest * paymentRequest = dynamic_cast<const PaymentRequest*>(&aRequest);
	if (!paymentRequest)
	{
		return;
	}

	if (getError() != EServerError::Ok)
	{
		return;
	}

	QVariant step = getParameter(CMultistage::Protocol::Step);
	if (step.isNull())
	{
		return;
	}

	mStep = step.toString();

	QVariant fields = getParameter(CMultistage::Protocol::Fields);
	if (fields.isNull() && mStep != CMultistage::Protocol::FinalStepValue)
	{
		return;
	}

	if (fields.isNull())
	{
		mFields = "";
		mIsOk = true;
	}
	else
	{
		mFields = QString::fromLocal8Bit(QByteArray::fromPercentEncoding(fields.toString().toLatin1())).trimmed();

		QDomDocument doc("mydocument");

		mIsOk = mFields.isEmpty() ? true : doc.setContent(mFields);
	}
}

//---------------------------------------------------------------------------
bool MultistagePaymentGetStepResponse::isOk()
{
	return ((getError() == EServerError::Ok) && mIsOk) ? true : false;
}

//---------------------------------------------------------------------------
QString MultistagePaymentGetStepResponse::getMultistageStep() const
{
	return mStep;
}

//---------------------------------------------------------------------------
QString MultistagePaymentGetStepResponse::getStepFields() const
{
	return mFields;
}

//---------------------------------------------------------------------------
