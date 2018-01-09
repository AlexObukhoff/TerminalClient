/* @file Базовый платёж через процессинг Киберплат. */

#pragma once

// Modules
#include <Payment/PaymentBase.h>

// Project
#include "PaymentFactory.h"


//------------------------------------------------------------------------------
class AdPayment : public PaymentBase
{
	friend class PaymentFactory;

public:
	AdPayment(PaymentFactory * aFactory);

	#pragma region SDK::PaymentProcessor::IPayment interface

	/// Если предикат возвращает true, то платёж не сформирован должным образом и может быть удалён.
	virtual bool isNull() const;

	/// Возвращает true, если платёж можно провести в оффлайне.
	virtual bool canProcessOffline() const;

	/// Выполнение шага с идентификатором aStep.
	virtual bool performStep(int aStep);

	/// Обновление статуса платежа.
	virtual void process();

	/// Отмена платёжа. В случае успеха возвращает true.
	virtual bool cancel();

	/// Отметка платежа как удаленного. В случае успеха возвращает true.
	virtual bool remove();

	#pragma endregion

	/// Возвращает связанную фабрику платежей.
	PaymentFactory * getPaymentFactory() const;

protected:
	/// Возвращает true, если ограничения на сумму платежа зависят от переданного параметра.
	virtual bool limitsDependOnParameter(const SParameter & aParameter);

	/// Подсчёт верхней и нижней границ для сумм платежа.
	virtual bool calculateLimits();

	/// Рассчитывает сумму платежа по полю AMOUNT_ALL.
	virtual void calculateSums();

	/// Создаёт класс запроса по идентификатору шага.
	virtual Request * createRequest(const QString & aStep);

	/// Создаёт класс ответа по классу запроса.
	virtual Response * createResponse(const Request & aRequest, const QString & aResponseString);

	/// Отправка запроса.
	virtual Response * sendRequest(const QUrl & aUrl, Request & aRequest);

	/// Транзакция.
	virtual bool pay();

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
