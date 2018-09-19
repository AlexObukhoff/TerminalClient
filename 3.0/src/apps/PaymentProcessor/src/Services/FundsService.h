/* @file Сервис для работы с устройствами приема наличных. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/IFundsService.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/IServiceState.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/Plugins/IPlugin.h>

// Modules
#include <Common/ILogable.h>

class IApplication;
class CashDispenserManager;
class CashAcceptorManager;

namespace CFundsService
{
	const char LogName[] = "Funds";
}

//---------------------------------------------------------------------------
class FundsService :
	public QObject, 
	public SDK::PaymentProcessor::IFundsService, 
	public SDK::PaymentProcessor::IService, 
	public SDK::PaymentProcessor::IServiceState,
	private ILogable	
{
	Q_OBJECT

public:
	/// Получение экземпляра FundsService.
	static FundsService * instance(IApplication * aApplication);

	FundsService(IApplication * aApplication);
	virtual ~FundsService();

	/// Инициализация сервиса.
	virtual bool initialize();

	/// IService: Закончена инициализация всех сервисов.
	virtual void finishInitialize();

	/// Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	/// Остановка сервиса.
	virtual bool shutdown();

	/// Имя сервиса.
	virtual QString getName() const;

	/// Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// Получение списка зависимостей.
	virtual const QSet<QString> & getRequiredServices() const;

	/// Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);

	/// Возвращает уникальный набор параметров для устройств с денежными средствами
	virtual QString getState() const;

public:
	/// Получить интерфейс для работы с источниками денег.
	virtual SDK::PaymentProcessor::ICashAcceptorManager * getAcceptor() const;

	/// Получить интерфейс для работы с устройствами выдачи денег.
	virtual SDK::PaymentProcessor::ICashDispenserManager * getDispenser() const;

private:
	IApplication * mApplication;
	CashDispenserManager * mCashDispenserManager;
	CashAcceptorManager * mCashAcceptorManager;
};

//---------------------------------------------------------------------------
