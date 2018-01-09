/* @file Базовый платёж через процессинг Киберплат. */

#pragma once

// Modules
#include <Payment/PaymentBase.h>

// Project
#include "PaymentFactory.h"

//------------------------------------------------------------------------------
class Payment : public PaymentBase
{
	friend class PaymentFactory;

public:
	Payment(PaymentFactory * aFactory);

	/// Возвращает связанную фабрику платежей.
	PaymentFactory * getPaymentFactory() const;

#pragma region SDK::PaymentProcessor::IPayment interface

	/// Выполнение шага с идентификатором aStep.
	virtual bool performStep(int aStep);

	/// Обновление статуса платежа.
	virtual void process();

	/// Отметка платежа как удаленного. В случае успеха возвращает true.
	virtual bool remove();

#pragma endregion

protected:
	/// Создаёт класс запроса по идентификатору шага.
	virtual Request * createRequest(const QString & aStep);

	/// Создаёт класс ответа по классу запроса.
	virtual Response * createResponse(const Request & aRequest, const QString & aResponseString);

	/// Отправка запроса.
	virtual Response * sendRequest(const QUrl & aUrl, Request & aRequest);

	/// Запрос на проведение платежа.
	virtual bool check(bool aFakeCheck);

	/// Транзакция.
	virtual bool pay();

	/// Запрос статуса платежа.
	virtual bool status();

	/// Попытка проведения платежа.
	virtual void performTransaction();

	/// Критичекая ошибка, проведение платежа прекращается.
	virtual bool isCriticalError(int aError) const;

	/// При ошибке проведения устанавливает таймауты для следующей попытки.
	virtual void setProcessError();

protected:
	RequestSender mRequestSender;
};

//------------------------------------------------------------------------------
