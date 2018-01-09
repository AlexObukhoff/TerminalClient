/* @file Вспомогательные методы для работы с платежами в БД. */

#pragma once

// SDK
#include <SDK/PaymentProcessor/Core/Encashment.h>
#include <SDK/PaymentProcessor/Payment/IPayment.h>
#include <SDK/PaymentProcessor/Payment/Amount.h>
#include <SDK/PaymentProcessor/Payment/Step.h>

//---------------------------------------------------------------------------
typedef QList<SDK::PaymentProcessor::IPayment::SParameter> TPaymentParameters;

//---------------------------------------------------------------------------
class IPaymentDatabaseUtils
{
public:
	/// Создание пустой платёжной записи в базе.
	virtual qint64 createDummyPayment() = 0;

	/// Возвращает идентификатор платежа по начальной сесии.
	virtual qint64 getPaymentByInitialSession(const QString & aInitialSession) = 0;

	/// Возвращает список параметров для платежа с идентификатором aId.
	virtual TPaymentParameters getPaymentParameters(qint64 aId) = 0;

	/// Возвращает список параметров для платежа с идентификаторами aIds.
	virtual QMap<qint64, TPaymentParameters> getPaymentParameters(const QList<qint64> & aIds) = 0;

	/// Сохраняет платёж в базе. Опционально можно указать подпись.
	virtual bool savePayment(SDK::PaymentProcessor::IPayment * aPayment, const QString & aSignature) = 0;

	/// Удаляет платёж из базы.
	virtual void removePayment(qint64 aPayment) = 0;

	/// Заморозить обработку платежа на aMinutes минут.
	virtual bool suspendPayment(qint64 aPayment, int aMinutes) = 0;

	/// Добавляет сумму aAmount к платежу с идентификатором aPayment.
	virtual bool addPaymentNote(qint64 aPayment, const SDK::PaymentProcessor::SNote & aNote) = 0;
	virtual bool addPaymentNote(qint64 aPayment, const QList<SDK::PaymentProcessor::SNote> & aNotes) = 0;

	/// Добавим в БД информацию о выданных купюрах
	virtual bool addChangeNote(const QString & aSession, const QList<SDK::PaymentProcessor::SNote> & aNotes) = 0;

	/// Получить информацию по всем купюорам в контексте платежа.
	virtual QList<SDK::PaymentProcessor::SNote> getPaymentNotes(qint64 aPayment) = 0;

	/// Возвращает список платежей, ожидающих проведения.
	virtual QList<qint64> getPaymentQueue() = 0;

	/// Возвращает краткую информацию по платежам и купюрам с последней инкассации.
	virtual SDK::PaymentProcessor::SBalance getBalance() = 0;

	/// Получить список платежей определенного статуса. В случае пустого списка статусов - получим все платежи из базы
	virtual QList<qint64> getPayments(const QSet<SDK::PaymentProcessor::EPaymentStatus::Enum> & aStates) = 0;

	/// Поиск платежа по номеру/счету
	virtual QList<qint64> findPayments(const QDate & aDate, const QString & aPhoneNumber) = 0;

	/// Выполняет инкассацию.
	virtual SDK::PaymentProcessor::SEncashment performEncashment(const QVariantMap & aParameters) = 0;

	/// Возвращает последнюю выполненную инкасацию 
	virtual QList<SDK::PaymentProcessor::SEncashment> getLastEncashments(int aCount) = 0;

	/// Выполняет архивацию устаревших платежей.
	virtual bool backupOldPayments() = 0;

	/// Получить кол-во платежей по каждому использованному провайдеру
	virtual QMap<qint64, quint32> getStatistic() const = 0;

protected:
	virtual ~IPaymentDatabaseUtils() {}
};

//---------------------------------------------------------------------------
