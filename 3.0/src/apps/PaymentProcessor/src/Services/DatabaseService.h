/* @file Сервиса, владеющий клиентом БД. */

#pragma once

// SDK
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/IDatabaseService.h>

// Проект
#include "DatabaseUtils/IDatabaseUtils.h"
#include "DatabaseProxy/IDatabaseProxy.h"

class IApplication;
class IDatabaseProxy;

//---------------------------------------------------------------------------
class DatabaseService : public SDK::PaymentProcessor::IDatabaseService, public SDK::PaymentProcessor::IService, public IDatabaseQueryChecker
{
public:
	/// Получение экземпляра DatabaseService.
	static DatabaseService * instance(IApplication * aApplication);

	DatabaseService(IApplication * aApplication);

	#pragma region SDK::PaymentProcessor::IService interface

	/// IService: Инициализация сервиса.
	virtual bool initialize();

	/// IService: Закончена инициализация всех сервисов.
	virtual void finishInitialize();

	/// IService: Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	/// IService: Завершение работы сервиса.
	virtual bool shutdown();

	/// IService: Возвращает имя сервиса.
	virtual QString getName() const;

	/// Получение списка необходимых сервисов.
	virtual const QSet<QString> & getRequiredServices() const;

	/// Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);

	#pragma endregion

	#pragma region SDK::PaymentProcessor::IDatabaseService interface

	/// Выполнение запроса по строке.
	virtual bool execQuery(const QString & aQuery);

	/// Подготавливает запрос к биндингу параметров.
	virtual QSharedPointer<IDatabaseQuery> prepareQuery(const QString & aQuery);

	/// Создание запроса по строке и его выполнение.
	virtual QSharedPointer<IDatabaseQuery> createAndExecQuery(const QString & aQuery);

	/// Выполнение переданного запроса.
	virtual bool execQuery(QSharedPointer<IDatabaseQuery> aQuery);

	#pragma endregion

	#pragma region SDK::PaymentProcessor::IDatabaseErrorChecker interface

	/// Проверка на ошибки при выполнении запросов к БД
	bool isGood(bool aQueryResult);

	#pragma endregion

	/// Получение нужной части интерфейса базы данных.
	template< typename T >
	T * getDatabaseUtils()
	{
		return dynamic_cast<T *>(mDbUtils.data());
	}

protected:
	virtual ~DatabaseService();

private:
	IApplication   * mApplication;
	IDatabaseProxy * mDatabase;
	QSharedPointer<IDatabaseUtils> mDbUtils;
	int mErrorCounter;
};

//---------------------------------------------------------------------------
