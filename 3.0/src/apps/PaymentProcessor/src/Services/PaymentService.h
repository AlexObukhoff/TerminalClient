/* @file Сервис, владеющий платёжными потоками. */

#pragma once

// C++
#include <functional>
#include <memory>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QFutureSynchronizer>
#include <QtCore/QAtomicInt>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>

// SDK
#include <SDK/PaymentProcessor/Payment/IPaymentFactory.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>

#include "DatabaseUtils/IPaymentDatabaseUtils.h"

namespace PPSDK = SDK::PaymentProcessor;

class IApplication;
class IPaymentDatabaseUtils;

//---------------------------------------------------------------------------
namespace EPaymentCommandResult
{
	enum Enum
	{
		OK,
		Error,
		NotFound
	};
}

Q_DECLARE_METATYPE(EPaymentCommandResult::Enum)

//---------------------------------------------------------------------------
class PaymentService : public PPSDK::IPaymentService, public PPSDK::IService, protected ILogable
{
	Q_OBJECT

public:
	static PaymentService * instance(IApplication * aApplication);

	PaymentService(IApplication * aApplication);

	#pragma region PPSDK::IService interface

	/// Инициализация сервиса.
	virtual bool initialize();

	/// IService: Закончена инициализация всех сервисов.
	virtual void finishInitialize();

	/// IService: Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	/// Завершение работы сервиса.
	virtual bool shutdown();

	/// Возвращает имя сервиса.
	virtual QString getName() const;

	/// Список зависимостей.
	virtual const QSet<QString> & getRequiredServices() const;

	/// Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);

	#pragma endregion

	#pragma region PPSDK::IPaymentService interface

	/// Создание платежа по номеру оператора aOperator. Возвращает номер платежа или код ошибки (меньше 0).
	virtual qint64 createPayment(qint64 aProvider);

	/// Возвращает номер активного платежа.
	virtual qint64 getActivePayment() const;

	/// Сбросить платёж из активного состояния
	virtual void deactivatePayment();

	/// Возвращает описание оператора.
	virtual PPSDK::SProvider getProvider(qint64 aID);

	/// Получение значения поля платежа.
	virtual PPSDK::IPayment::SParameter getPaymentField(qint64 aPayment, const QString & aName);

	/// Вычислить размер комиссии для активного платежа, без записи результата в параметры платежа
	virtual QList<PPSDK::IPayment::SParameter> calculateCommission(const QList<PPSDK::IPayment::SParameter> & aParameters);

	/// Поиск платежа по номеру/счету
	virtual QList<qint64> findPayments(const QDate & aDate, const QString & aPhoneNumber);

	/// Получение списка полей платежа.
	virtual QList<PPSDK::IPayment::SParameter> getPaymentFields(qint64 aPayment);

	/// Получение списка полей платежей.
	virtual QMap<qint64, QList<SDK::PaymentProcessor::IPayment::SParameter> > getPaymentsFields(const QList<qint64> & aIds);

	/// Обновление поля платежа.
	virtual bool updatePaymentField(qint64 aPayment, const PPSDK::IPayment::SParameter & aField, bool aForce = false);

	/// Обновление полей платежа.
	virtual bool updatePaymentFields(qint64 aPayment, const QList<PPSDK::IPayment::SParameter> & aFields, bool aForce = false);

	/// Выполенение шага платежа в онлайне.
	virtual void processPaymentStep(qint64 aPayment, PPSDK::EPaymentStep::Enum aStep, bool aBlocking = false);

	/// Конвертация переданного платежа к типу aTargetType, поддерживаему этой фабрикой. В случае ошибки возвращает false.
	virtual bool convertPayment(qint64 aPayment, const QString & aTargetType);

	/// Проведение платежа. Если aOnline - false, то платёж будет обработан в порядке общей очереди,
	/// иначе в реальном времени. Функция не блокирующая, результат online проведения придёт
	/// в сигнале stepCompleted.
	virtual bool processPayment(qint64 aPayment, bool aOnline);

	/// Отменяет платёж если это возможно.
	virtual bool cancelPayment(qint64 aPayment);

	/// Останавливает платеж с текстом ошибки
	virtual bool stopPayment(qint64 aPayment, int aError, const QString & aErrorMessage);

	/// Пометить платёж как удаленный, если это возможно.
	virtual bool removePayment(qint64 aPayment);

	/// Возвращает true, если платёж можно провести в оффлайне.
	virtual bool canProcessPaymentOffline(qint64 aPayment);

	/// Разбудить процесс проведения платежей (для быстрого проведения)
	virtual void hangupProcessing();

	/// Возвращает размер доступной для использования сдачи.
	virtual double getChangeAmount();

	/// Сумма сдачи переводится в пользу платежа aPayment.
	virtual void moveChangeToPayment(qint64 aPayment);

	/// Сумма платежа aPayment переводится в сдачу.
	virtual void movePaymentToChange(qint64 aPayment);

	/// Сбрасывает счётчик со сдачей от предыдущих плетежей.
	virtual void resetChange();

	/// Получить сессию платежа, с которого осталась сдача
	virtual QString getChangeSessionRef();

	/// Возвращает краткую информацию о суммах и платежах с момента последней инкассации.
	virtual PPSDK::SBalance getBalance();

	/// Получить список платежей определенного статуса. В случае пустого списка статусов - получим все платежи из базы
	virtual QList<qint64> getPayments(const QSet<PPSDK::EPaymentStatus::Enum> & aStates);

	/// Проведение инкассации.
	/// В БД формирует отчёт о всех принятых за период платежах в формате:
	/// <ID>\t<дата_создания>\t<начальная_сессия>\t<сессия>\t<провайдер>\t<сумма_платежа>\t<принятая_сумма>\t<статус>\t<поля>\t<купюры>\r\n
	/// Дополнительно: <поля> хранятся в UTF-8, <имя>:<значение>|<имя>:<значение>...
	///                <купюры> хранятся в формате <номинал>:<количество>|<номинал>:<количество>...
	virtual PPSDK::EncashmentResult::Enum performEncashment(const QVariantMap & aParameters, PPSDK::SEncashment & aEncashment);

	/// Получение информации о последней инкасации
	virtual PPSDK::SEncashment getLastEncashment();

	/// Получение информации о последних инкасациях
	virtual QList<PPSDK::SEncashment> getEncashmentList(int aDepth);

	/// Получить кол-во платежей по каждому использованному провайдеру
	virtual QMap<qint64, quint32> getStatistic() const;

	/// Получение списка купюр для платежа с идентификатором aID.
	virtual QList<SDK::PaymentProcessor::SNote> getPaymentNotes(qint64 aID) const;

	#pragma endregion

	/// Добавляет команду на изменение параметров платежа в очередь. Если возвращает 0, то команда не может
	/// быть выполнена. После завершения обработки, срабатывает сигнал paymentCommandComplete.
	/// Поиск платежа осуществляется по начальной сессии.
	virtual int registerForcePaymentCommand(const QString & aInitialSession, const QVariantMap & aParameters);

	/// Добавляет команду на пометку как удаленного платежа в очередь. Если возвращает 0, то команда не может
	/// быть выполнена. После завершения обработки, срабатывает сигнал paymentCommandComplete.
	/// Поиск платежа осуществляется по начальной сессии.
	virtual int registerRemovePaymentCommand(const QString & aInitialSession);

protected:
	/// Формирование подписи платежа.
	QString createSignature(PPSDK::IPayment * aPayment, bool aWithCRC = true);

	/// Проверка подписи для платежа.
	bool verifySignature(PPSDK::IPayment * aPayment);

	/// Загружает объект платежа по идентификатору.
	std::shared_ptr<PPSDK::IPayment> getPayment(qint64 aID);

	/// Загружает объект платежа по начальной сессии.
	std::shared_ptr<PPSDK::IPayment> getPayment(const QString & aInitialSession);

	/// Функция сохранения платежа.
	bool savePayment(PPSDK::IPayment * aPayment);

private:
	/// Добавляет платёж в список активных.
	void setPaymentActive(std::shared_ptr<PPSDK::IPayment> aPayment);

	/// Если указанный платёж сейчас активен, возвращает true.
	bool isPaymentActive(std::shared_ptr<PPSDK::IPayment> aPayment);

	/// Сохраняет сдачу.
	bool setChangeAmount(double aChange, std::shared_ptr<PPSDK::IPayment> aPaymentSource);

	/// Обновляет параметры платежа.
	void doUpdatePaymentFields(qint64 aID, std::shared_ptr<PPSDK::IPayment> aPayment, const QList<SDK::PaymentProcessor::IPayment::SParameter> & aFields, bool aForce = false);

signals:
	/// Сигнал о завершении обработки платёжной команды, добавленной методом registerPaymentCommand.
	void paymentCommandComplete(int aID, EPaymentCommandResult::Enum aError);

	/// Обновление суммы сдачи
	void changeUpdated(double aAmount);

private slots:
	/// Поток для обработки оффлайн платежей запущен.
	void onPaymentThreadStarted();

	/// Проведение накопившихся платежей.
	void onProcessPayments();

	/// Обработка сигнала о приёме денежных средств.
	void onAmountUpdated(qint64 aPayment, double aTotalAmount, double aAmount);

	/// Обработка сигнала о выдаче денежных средств.
	void onAmountDispensed(double aAmount);

private:
	/// Проведение платежа
	bool processPaymentInternal(std::shared_ptr<PPSDK::IPayment> aPayment);

private:
	IApplication * mApplication;
	volatile bool mEnabled;
	IPaymentDatabaseUtils * mDBUtils;

	QList<PPSDK::IPaymentFactory *> mFactories;
	QMap<QString, PPSDK::IPaymentFactory *> mFactoryByType;

	QMutex mPaymentLock;

	/// Активный платёж.
	std::shared_ptr<PPSDK::IPayment> mActivePayment;

	/// Платёж, в котором хранится неизрасходованная сдача.
	std::shared_ptr<PPSDK::IPayment> mChangePayment;

	QFutureSynchronizer<void> mActivePaymentSynchronizer;

	QThread mPaymentThread;
	QTimer mPaymentTimer;

	QMutex mCommandMutex;

	/// Порядковый номер команды на изменение/удаление платежа.
	int mCommandIndex;

	/// Очередь команд на изменение/удаление платежа.
	QList<QPair<int, std::function<EPaymentCommandResult::Enum(PaymentService *)>>> mCommands;
	
	/// Список платежей, имеющих несохраненные параметры
	QSet<qint64> mPaymentHaveUnsavedParameters;

	/// Дата последней выгрузки устаревших платежей.
	QDateTime mLastBackupDate;

	/// ID платежа, проходящего в данный момент обработку в оффлайне.
	QMutex mOfflinePaymentLock;
	qint64 mOfflinePaymentID;
	std::shared_ptr<PPSDK::IPayment> mOfflinePayment;
};

//---------------------------------------------------------------------------

