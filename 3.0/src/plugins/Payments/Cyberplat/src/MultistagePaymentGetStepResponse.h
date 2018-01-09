/* @file Ответ сервера на получения полей для следующего шага multistage шлюза. */

#pragma once

// Project
#include "PaymentResponse.h"

//---------------------------------------------------------------------------
class MultistagePaymentGetStepResponse : public PaymentResponse
{
public:
	MultistagePaymentGetStepResponse(const Request & aRequest, const QString & aResponseString);

	/// Response: Предикат истинен, если ответ сервера не содержит ошибок.
	virtual bool isOk();

	virtual QString getMultistageStep() const;
	virtual QString getStepFields() const;

private:
	QString mFields;
	QString mStep;
	bool mIsOk;
};

//---------------------------------------------------------------------------
