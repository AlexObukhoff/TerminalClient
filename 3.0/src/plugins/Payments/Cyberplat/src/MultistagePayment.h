/* @file Многошаговый платёж через процессинг Киберплат. */

#pragma once

// Project
#include "Payment.h"


//------------------------------------------------------------------------------
class MultistagePayment : public Payment
{
public:
	MultistagePayment(PaymentFactory * aFactory);

	#pragma region SDK::PaymentProcessor::IPayment interface

	/// Возвращает true, если платёж можно провести в оффлайне.
	virtual bool canProcessOffline() const;

	/// Выполнение шага с идентификатором aStep.
	virtual bool performStep(int aStep);

	#pragma endregion

	/// Получить номер текущего шага
	QString currentStep() const;

	/// Признак, что текущий шаг - первый
	bool isFirstStep() const;

	/// Признак, что текущий шаг - последний
	bool isFinalStep() const;

	/// Получить список полей для конкретного шага, если шаг не существует - вернет пустой список
	SDK::PaymentProcessor::TProviderFields getFieldsForStep(const QString & aStep) const;

	/// Получить список предыдущих шагов
	QStringList getHistory();

	/// Выполнить запрос перехода на следующий шаг
	bool getNextStep();

protected:
	/// Создаёт класс запроса по идентификатору шага.
	virtual Request * createRequest(const QString & aStep);

	/// Создаёт класс ответа по классу запроса.
	virtual Response * createResponse(const Request & aRequest, const QString & aResponseString);

	/// Разобрать xml с ответом сервера, содержащим поля для конкретного шага.
	SDK::PaymentProcessor::TProviderFields parseFieldsXml(const QString & aFieldsXml) const;

	/// Записать новые параметры в платеж
	void nextStep(const QString & aStep, const QString & aFields);
};

//---------------------------------------------------------------------------
