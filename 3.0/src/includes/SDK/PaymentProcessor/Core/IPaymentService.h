/* @file Интерфейс обеспечивающей взаимодействие с платежами. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/Encashment.h>
#include <SDK/PaymentProcessor/Payment/IPayment.h>
#include <SDK/PaymentProcessor/Payment/Amount.h>
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/Settings/Provider.h>

namespace SDK {
namespace PaymentProcessor {

namespace EncashmentResult
{
	enum Enum
	{
		OK, 
		Error, 
		TryLater 
	};
}

namespace EncashmentParameter
{
	const char StackerID[] = "STACKER_ID";
}

//------------------------------------------------------------------------------
class IPaymentService : public QObject
{
	Q_OBJECT

public:
	/// Создание платежа по номеру оператора aOperator. Возвращает номер платежа или код ошибки (меньше 0).
	virtual qint64 createPayment(qint64 aOperator) = 0;

	/// Возвращает номер активного платежа.
	virtual qint64 getActivePayment() const = 0;

	/// Сбросить платёж из активного состояния
	virtual void deactivatePayment() = 0;

	/// Возвращает описание оператора.
	virtual SProvider getProvider(qint64 aID) = 0;

	/// Получение значения поля платежа.
	virtual IPayment::SParameter getPaymentField(qint64 aPayment, const QString & aName) = 0;

	/// Получение списка полей платежа.
	virtual QList<IPayment::SParameter> getPaymentFields(qint64 aPayment) = 0;

	/// Вычислить размер комиссии для активного платежа, без записи результата в параметры платежа
	virtual QList<IPayment::SParameter> calculateCommission(const QList<IPayment::SParameter> & aParameters) = 0;

	/// Поиск платежа по номеру/счету
	virtual QList<qint64> findPayments(const QDate & aDate, const QString & aPhoneNumber) = 0;

	/// Получение списка полей платежей.
	virtual QMap<qint64, QList<SDK::PaymentProcessor::IPayment::SParameter> > getPaymentsFields(const QList<qint64> & aIds) = 0;

	/// Обновление поля платежа.
	virtual bool updatePaymentField(qint64 aPayment, const IPayment::SParameter & aField, bool aForceUpdate = false) = 0;

	/// Обновление полей платежа.
	virtual bool updatePaymentFields(qint64 aPayment, const QList<IPayment::SParameter> & aFields, bool aForceUpdate = false) = 0;

	/// Выполенение шага платежа в онлайне.
	virtual void processPaymentStep(qint64 aPayment, EPaymentStep::Enum aStep, bool aBlocking = false) = 0;

	/// Конвертация переданного платежа к типу aTargetType, поддерживаему этой фабрикой. В случае ошибки возвращает false.
	virtual bool convertPayment(qint64 aPayment, const QString & aTargetType) = 0;

	/// Проведение платежа. Если aOnline - false, то платёж будет обработан в порядке общей очереди,
	/// иначе в реальном времени. Функция не блокирующая, результат online проведения придёт
	/// в сигнале stepCompleted.
	virtual bool processPayment(qint64 aPayment, bool aOnline) = 0;

	/// Отменяет платёж если это возможно.
	virtual bool cancelPayment(qint64 aPayment) = 0;

	/// Останавливает платеж с текстом ошибки
	virtual bool stopPayment(qint64 aPayment, int aError, const QString & aErrorMessage) = 0;

	/// Пометить платёж как удаленный, если это возможно.
	virtual bool removePayment(qint64 aPayment) = 0;

	/// Возвращает true, если платёж можно провести в оффлайне.
	virtual bool canProcessPaymentOffline(qint64 aPayment) = 0;

	/// Разбудить процесс проведения платежей (для быстрого проведения)
	virtual void hangupProcessing() = 0;

	/// Возвращает размер доступной для использования сдачи.
	virtual double getChangeAmount() = 0;

	/// Сумма сдачи переводится в пользу платежа aPayment.
	virtual void moveChangeToPayment(qint64 aPayment) = 0;

	/// Сумма платежа aPayment переводится в сдачу.
	virtual void movePaymentToChange(qint64 aPayment) = 0;

	/// Сбрасывает счётчик со сдачей от предыдущих плетежей.
	virtual void resetChange() = 0;

	/// Получить сессию платежа, с которого осталась сдача
	virtual QString getChangeSessionRef() = 0;

	/// Возвращает краткую информацию о суммах и платежах с момента последней инкассации.
	virtual SBalance getBalance() = 0;

	/// Получить список платежей определенного статуса
	virtual QList<qint64> getPayments(const QSet<EPaymentStatus::Enum> & aStates) = 0;

	/// Возвращает информацию о принятых купюрах в контексте платежа.
	virtual QList<SNote> getPaymentNotes(qint64 aID) const = 0;

	/// Проведение инкассации.
	/// В БД формирует отчёт о всех принятых за период платежах в формате:
	/// <ID>\t<дата_создания>\t<начальная_сессия>\t<сессия>\t<провайдер>\t<сумма_платежа>\t<принятая_сумма>\t<статус>\t<поля>\t<купюры>\r\n
	/// Дополнительно: <поля> хранятся в UTF-8, <имя>:<значение>|<имя>:<значение>...
	///                <купюры> хранятся в формате <номинал>:<количество>|<номинал>:<количество>...
	virtual EncashmentResult::Enum performEncashment(const QVariantMap & aParameters, SEncashment & aEncashment) = 0;

	/// Получение информации о последней инкасации
	virtual SEncashment getLastEncashment() = 0;

	/// Получение информации о последних инкасациях
	virtual QList<SEncashment> getEncashmentList(int aDepth) = 0;

	/// Получить кол-во платежей по каждому использованному провайдеру
	virtual QMap<qint64, quint32> getStatistic() const = 0;

signals:
	/// Сигнал об обновлении сумм платежа с указанным идентификатором.
	void amountUpdated(qint64 aPayment);

	/// Сигнал о завершении выполнения шага платежа.
	void stepCompleted(qint64 aPayment, int aStep, bool aError);

protected:
	virtual ~IPaymentService() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor

