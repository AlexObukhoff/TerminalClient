/* @file Пиновый платёж через процессинг Киберплат. */

#pragma once

// Project
#include "Payment.h"

//------------------------------------------------------------------------------
namespace CPin
{
	const char UIFieldName[] = "CARD";
}


//------------------------------------------------------------------------------
class PinPayment : public Payment
{
public:
	PinPayment(PaymentFactory * aFactory);

	#pragma region SDK::PaymentProcessor::IPayment interface

	/// Возвращает true, если платёж можно провести в оффлайне.
	virtual bool canProcessOffline() const;

	#pragma endregion

protected:
	/// Получение лимитов для данного платежа
	virtual bool getLimits(double & aMinAmount, double & aMaxAmount);

	/// Возвращает true, если ограничения на сумму платежа зависят от переданного параметра.
	virtual bool limitsDependOnParameter(const SParameter & aParameter);

protected:
	/// Создаёт класс запроса по идентификатору шага.
	virtual Request * createRequest(const QString & aStep);
};

//---------------------------------------------------------------------------
