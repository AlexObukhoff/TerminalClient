/* @file Реализация интерфейсов для работы с БД. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMutex>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILog.h>

// Project
#include "DatabaseUtils/IDatabaseUtils.h"
#include "DatabaseUtils/IHardwareDatabaseUtils.h"
#include "DatabaseUtils/IPaymentDatabaseUtils.h"

//---------------------------------------------------------------------------
class IDatabaseProxy;
class IApplication;

//---------------------------------------------------------------------------
class DatabaseUtils : public IDatabaseUtils,
                      public IHardwareDatabaseUtils,
                      public IPaymentDatabaseUtils
{
public:
	DatabaseUtils(IDatabaseProxy & aProxy, IApplication * aApplication);
	virtual ~DatabaseUtils();

	/// Инициализация.
	virtual bool initialize();

	#pragma region IDatabaseUtils interface

	/// Подготавливает к выполнению запрос.
	virtual IDatabaseQuery * prepareQuery(const QString & aQuery);

	/// Потокозащищённое выполнение произвольного запроса.
	virtual bool execQuery(IDatabaseQuery * aQuery);

	/// Освобождение памяти из-под запроса.
	virtual void releaseQuery(IDatabaseQuery * aQuery);

	#pragma endregion

	#pragma region IHardwareDatabaseUtils interface

	/// Возвращает список параметров устройства по имени конфигурации устройства.
	virtual bool getDeviceParams(const QString & aDeviceConfigName, QVariantMap & aParameters);

	/// Возвращает true, если параметр aName для устройства с именем aDeviceName и типом aType существует.
	virtual bool isDeviceParamExists(const QString & aDeviceConfigName);

	/// Возвращает значение конкретного параметра из список параметров устройства по имени и типу.
	virtual QVariant getDeviceParam(const QString & aDeviceConfigName, const QString& aName);

	/// Добавить определенный параметр устройства по имени и типу.
	virtual bool setDeviceParam(const QString & aDeviceConfigName, const QString & aParamName, const QVariant & aParamValue);

	/// Предикат, пределяющий наличие устройства по его имени и типу.
	virtual bool hasDevice(const QString & aDeviceConfigName);

	/// Добавить новый девайс.
	virtual bool addDevice(const QString & aDeviceConfigName);

	/// Удаление устройств aDevice с типом aType.
	virtual bool removeDeviceParams(const QString & aDeviceConfigName);

	/// Удаляем отправленые статусы устройств.
	virtual bool cleanDevicesStatuses();

	/// Удалить из базы неиспользуемые конфигурации устойств
	virtual void removeUnknownDevice(const QStringList & aCurrentDevicesList);

	/// Вставить новый статус девайсов.
	virtual bool addDeviceStatus(const QString & aDeviceConfigPath, SDK::Driver::EWarningLevel::Enum aErrorLevel, const QString & aStatusString);

	#pragma endregion

	#pragma region IPaymentDatabaseUtils interface

	/// Создание пустой платёжной записи в базе.
	virtual qint64 createDummyPayment();

	/// Возвращает идентификатор платежа по начальной сесии.
	virtual qint64 getPaymentByInitialSession(const QString & aInitialSession);

	/// Возвращает список параметров для платежа с идентификатором aId.
	virtual TPaymentParameters getPaymentParameters(qint64 aId);

	/// Возвращает список параметров для платежа с идентификаторами aIds.
	virtual QMap<qint64, TPaymentParameters> getPaymentParameters(const QList<qint64> & aIds);

	/// Сохраняет платёж в базе. Опционально можно указать подпись.
	virtual bool savePayment(SDK::PaymentProcessor::IPayment * aPayment, const QString & aSignature);

	/// Удаляет платёж из базы.
	virtual void removePayment(qint64 aPayment);

	/// Временно приостановить обработку платежа.
	virtual bool suspendPayment(qint64 aPayment, int aMinutes);

	/// Добавляет сумму aAmount к платежу с идентификатором aPayment.
	virtual bool addPaymentNote(qint64 aPayment, const SDK::PaymentProcessor::SNote & aNote);
	virtual bool addPaymentNote(qint64 aPayment, const QList<SDK::PaymentProcessor::SNote> & aNotes);

	/// Добавим в БД информацию о выданных купюрах
	virtual bool addChangeNote(const QString & aSession, const QList<SDK::PaymentProcessor::SNote> & aNotes);

	/// Получить информацию по всем купюорам в контексте платежа.
	virtual QList<SDK::PaymentProcessor::SNote> getPaymentNotes(qint64 aPayment);

	/// Возвращает список платежей, ожидающих проведения.
	virtual QList<qint64> getPaymentQueue();

	/// Возвращает краткую информацию по платежам и купюрам с последней инкассации.
	virtual SDK::PaymentProcessor::SBalance getBalance();

	/// Получить список платежей определенного статуса. В случае пустого списка статусов - получим все платежи из базы
	virtual QList<qint64> getPayments(const QSet<SDK::PaymentProcessor::EPaymentStatus::Enum> & aStates);

	/// Поиск платежа по номеру/счету
	virtual QList<qint64> findPayments(const QDate & aDate, const QString & aPhoneNumber);

	/// Выполняет инкассацию.
	virtual SDK::PaymentProcessor::SEncashment performEncashment(const QVariantMap & aParameters);

	/// Возвращает последнюю выполненную инкасацию 
	virtual QList<SDK::PaymentProcessor::SEncashment> getLastEncashments(int aCount);

	/// Выполняет архивацию устаревших платежей.
	virtual bool backupOldPayments();

	/// Получить кол-во платежей по каждому использованному провайдеру
	virtual QMap<qint64, quint32> getStatistic() const;

	#pragma endregion

private:
	IApplication * mApplication;
	IDatabaseProxy & mDatabase;
	ILog * mLog;
	ILog * mPaymentLog;
	QMutex mAccessMutex;

private:
	/// Заполняет отчет инкассации о платежах
	void fillEncashmentReport(SDK::PaymentProcessor::SEncashment & aEncashment);

private:
	/// Возвращает количество таблиц в базе
	int databaseTableCount() const;

	/// возвращает db_patch терминала
	int databasePatch() const;

	/// Выполнить обновление базы
	bool updateDatabase(const QString & aSqlScriptName);
};

//---------------------------------------------------------------------------
