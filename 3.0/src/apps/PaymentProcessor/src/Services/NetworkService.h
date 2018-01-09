/* @file Реализация менеджера сетевых соединений. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// Модули
#include <Common/ILogable.h>
#include <Connection/IConnection.h>
#include <NetworkTaskManager/NetworkTaskManager.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/Drivers/IModem.h>
#include <SDK/Drivers/IWatchdog.h>

// Проект
#include "System/IApplication.h"

class IHardwareDatabaseUtils;

//---------------------------------------------------------------------------
/// Менеджер соединений.
class NetworkService :
	protected QThread, 
	public SDK::PaymentProcessor::INetworkService, 
	public ILogable,
	public SDK::PaymentProcessor::IService
{
	Q_OBJECT

public:
	NetworkService(IApplication * aApplication);
	virtual ~NetworkService();

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

	/// Список зависимостей по сервисам.
	virtual const QSet<QString> & getRequiredServices() const;

	/// Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);

	/// Устанавливает соединение.
	virtual bool openConnection(bool aWait = true);

	/// Разрывает соединение.
	virtual bool closeConnection();

	/// Проверяет установленно ли соединение.
	virtual bool isConnected(bool aUseCache = false);

	/// Тестирует соединение: устанавливает, проверяет доступность ресурса aHost, разрывает.
	virtual bool testConnection();

	/// Получение интерфейса, обеспечивающего взаимодействие с сетью.
	virtual NetworkTaskManager * getNetworkTaskManager();

	/// Устанавливает параметры активного соединения.
	virtual void setConnection(const SDK::PaymentProcessor::SConnection & aConnection);

	/// Возвращает параметры активного соединения.
	virtual SDK::PaymentProcessor::SConnection getConnection() const;

	/// Возвращает последнюю ошибку соединения
	virtual QString getLastConnectionError() const;

	/// Устанавливает User-Agent для http запросов
	virtual void setUserAgent(const QString aUserAgent);

	/// Получает установленный User-Agent для http запросов
	virtual QString getUserAgent() const;

protected:
	/// QThread.
	virtual void run();

private slots:
	void onModemInitialized();
	void doConnect(const SDK::PaymentProcessor::SConnection & aConnection);
	bool doDisconnect();
	void doTestConnection(bool * aResult);
	/// Проверка есть ли связь, и попытаться поднять, если ее нет.
	void checkConnection();

private:
	SDK::Driver::IModem * getModem();
	SDK::Driver::IModem * prepareModem(SDK::Driver::IModem * aModemDevice, const QString & aConnectionName);
	bool getConnectionTemplate(const QString & aConnectionName, SDK::PaymentProcessor::SConnectionTemplate & aConnectionTem) const;

private slots:
	void onConnectionAlive();
	void onConnectionLost();
	void updateModemParameters();
	void onNetworkTaskStatus(bool aFailure);

private:
	SDK::PaymentProcessor::IDeviceService * mDeviceService;
	SDK::PaymentProcessor::IEventService * mEventService;
	QSharedPointer<IConnection> mConnection;

	/// Признак работы сетевого сервиса
	volatile bool mEnabled;

	// Число неудачных попыток соединения.
	int mFails;
	QTimer mRestoreTimer;

	SDK::PaymentProcessor::SConnection mConnectionSettings;

	mutable QMutex mErrorMutex;
	QString mLastConnectionError;

	/// Флаги сервиса.
	bool mDontWatchConnection;
	QTimer mParametersUpdateTimer;

	NetworkTaskManager mNetworkTaskManager;
	/// Время первого неудачного обращения по сети
	QDateTime mNetworkTaskFailureStamp;

	/// Параметры модема.
	boost::optional<QString> mBalance;
	boost::optional<int> mSignalLevel;
	boost::optional<QString> mOperator;

	IHardwareDatabaseUtils * mDatabaseUtils;
	IApplication * mApplication;
};

//---------------------------------------------------------------------------

