/* @file Инициализация и получение сервисов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QTimer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/Plugins/IExternalInterface.h>

//----------------------------------------------------------------------------
class IApplication;

namespace SDK {
	namespace Plugin {
		class IPlugin;
	}
}

//----------------------------------------------------------------------------
class ServiceController : public SDK::PaymentProcessor::ICore, public SDK::Plugin::IExternalInterface
{
	Q_OBJECT

public:
	ServiceController(IApplication * aApplication);
	virtual ~ServiceController();

	/// Регистрация нового сервиса.
	void registerService(SDK::PaymentProcessor::IService * aService);

	/// Запуск инициализации всех сервисов.
	bool initializeServices();

	/// Выводит отчет о выполнении инициализации в лог.
	void dumpFailureReport();

protected slots:
	/// Остановка всех сервисов.
	void shutdownServices();

	/// Реинициализация всех сервисов.
	void reinitializeServices();

	/// Перезагрузка терминала
	void rebootMachine();

	/// Перезапустить ТК
	void restartApplication();

	/// Выключение терминала
	void shutdownMachine();

	/// Обработчики событий WS.
	void onCloseCommandReceived();
	void onDisconnected();

protected slots:
	void onEvent(const SDK::PaymentProcessor::Event & aEvent);

signals:
	/// Сигнал к завершению работы.
	void exit(int aReturnCode);

protected:
	/// Завершение работы сервисов. В случае неудачи по таймауту вызывает слот aRetrySlot.
	bool finalizeServices(const char * aRetrySlot);

	/// Загрузить плагины ядра
	void initializeCoreItems();

	/// Удалить плагины ядра
	void finalizeCoreItems();

protected:
	#pragma region ICore interface

	/// ICore: возвращает интерфейс взаимодействия с платежами.
	virtual SDK::PaymentProcessor::IPaymentService * getPaymentService() const;

	/// ICode: получить список всех сервисов.
	virtual QSet<SDK::PaymentProcessor::IService *> getServices() const;

	/// Возвращает интерфейс взаимодействия с мониторингом.
	virtual SDK::PaymentProcessor::IRemoteService * getRemoteService() const;

	/// Возвращает интерфейс для приёма средств.
	virtual SDK::PaymentProcessor::IFundsService * getFundsService() const;

	/// Возвращает интерфейс печати.
	virtual SDK::PaymentProcessor::IPrinterService * getPrinterService() const;

	/// Возвращает интерфейс HID-устройств.
	virtual SDK::PaymentProcessor::IHIDService * getHIDService() const;

	/// Возвращает интерфейс для работы с сетевыми запросами.
	virtual SDK::PaymentProcessor::INetworkService * getNetworkService() const;

	/// Возвращает интефрейс для отправки событий объектам.
	virtual SDK::PaymentProcessor::IEventService * getEventService() const;

	/// Возвращает интерфейс для взаимодействия с графикой.
	virtual SDK::PaymentProcessor::IGUIService * getGUIService() const;

	/// Возвращает интерфейс для работы с устройствами.
	virtual SDK::PaymentProcessor::IDeviceService * getDeviceService() const;

	/// Возвращает криптографический интерфейс.
	virtual SDK::PaymentProcessor::ICryptService * getCryptService() const;

	/// Возвращает интерфейс для работы с настройками.
	virtual SDK::PaymentProcessor::ISettingsService * getSettingsService() const;

	/// Возвращает интерфейс для работы с базой данных.
	virtual SDK::PaymentProcessor::IDatabaseService * getDatabaseService() const;

	/// Возвращает интерфейс управления терминалом.
	virtual SDK::PaymentProcessor::ITerminalService * getTerminalService() const;

	/// Возвращает сервис с заданным именем.
	virtual SDK::PaymentProcessor::IService * getService(const QString & aServiceName) const;

	/// Читает пользовательскую переменную.
	virtual QVariantMap & getUserProperties();

	/// Возвращает false, если какой либо из сервисов не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	#pragma endregion

protected:
	/// Пользовательские переменные.
	QVariantMap mUserProperties;

	/// Список зарегистрированных сервисов.
	QMap<QString, SDK::PaymentProcessor::IService *> mRegisteredServices;

	/// Список инициализированных сервисов.
	QSet<QString> mInitializedServices;

	/// Имена сервисов, инициализация которых не удалась.
	QSet<QString> mFailedServices;

	/// Список сервисов в порядке их отключения.
	QList<SDK::PaymentProcessor::IService *> mShutdownOrder;

	/// Список загруженных плагинов ядра
	QList<SDK::Plugin::IPlugin *> mCorePluginList;

	IApplication * mApplication;

	/// Таймер остановки сервисов
	QTimer * mFinalizeTimer;

	/// Код выхода из приложения
	int mReturnCode;
};

//----------------------------------------------------------------------------

