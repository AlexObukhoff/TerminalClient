/* @file Менеджер, загружающий клиенты мониторинга. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMutex>
#include <QtCore/QTimer>
#include <QtCore/QStringList>
#include <QtCore/QSettings>
#include <QtCore/QtConcurrentRun>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>

// SDK
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/IRemoteClient.h>
#include <SDK/PaymentProcessor/Core/IRemoteService.h>
#include <SDK/PaymentProcessor/Core/IService.h>

// Project
#include "PaymentService.h"

namespace PPSDK = SDK::PaymentProcessor;

class IApplication;
class IHardwareDatabaseUtils;
class QFileSystemWatcher;

//---------------------------------------------------------------------------
class RemoteService : public SDK::PaymentProcessor::IRemoteService, public PPSDK::IService, protected ILogable
{
	Q_OBJECT

public:
	struct UpdateCommand
	{
		int ID;
		EStatus status;

		EUpdateType type;
		QUrl configUrl;
		QUrl updateUrl;
		QStringList parameters;
		QDateTime lastUpdate;

		UpdateCommand();
		bool isValid() const;

		/// Команда выполняется внешним updater.exe
		bool isExternal() const;
	};

public:
	RemoteService(IApplication * aApplication);
	virtual ~RemoteService();

	static RemoteService * instance(IApplication * aApplication);

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

	/// Сброс служебных параметров.
	virtual void resetParameters(const QSet<QString> & aParameters);

	#pragma endregion

	/// Попытка восстановление конфигурации терминала через один из подключенных клиентов мониторинга.
	/// Возращает true, если запрос на обновление конфигурации передан исполнителю.
	bool restoreConfiguration();

protected:
	#pragma region PPSDK::IRemoteService interface

	/// Добавляет в очередь команду на блокировку терминала.
	virtual int registerLockCommand();

	/// Добавляет в очередь команду на разблокировку терминала.
	virtual int registerUnlockCommand();

	/// Добавляет в очередь команду на перезагрузку терминала.
	virtual int registerRebootCommand();

	/// Добавляет в очередь команду на перезапуск ТК.
	virtual int registerRestartCommand();

	/// Добавляет в очередь команду на выключение терминала.
	virtual int registerShutdownCommand();

	/// Добавляет в очередь команду на изменение параметров платежа (поиск осуществляется по начальной сессии).
	virtual int registerPaymentCommand(EPaymentOperation aOperation, const QString & aInitialSession, const QVariantMap & aParameters);

	/// Добавляет в очередь команду на обновление файлов.
	virtual int registerUpdateCommand(EUpdateType aType, const QUrl & aConfigUrl, const QUrl & aUpdateUrl, const QString & aComponents);

	/// Добавляет в очередь команду на получение скриншота.
	virtual int registerScreenshotCommand();

	/// Добавляет в очередь команду на перегенерацию ключей
	virtual int registerGenerateKeyCommand(const QString & aLogin, const QString & aPassword);

	/// Зарегистрировать номер произвольной команды 
	virtual int registerAnyCommand();

	/// Сообщает сервису мониторинга что статус команды успешно отправлен
	virtual void commandStatusSent(int aCommandId, int aStatus);

	/// Требует от клиентов обновления контента.
	virtual void updateContent();

	/// Принудительная отправка сигнала heartbeat.
	virtual void sendHeartbeat();

	#pragma endregion

private:
	/// Выполнение команды с помощью делегирования событийной системе.
	int executeCommand(PPSDK::EEventType::Enum aEvent);

	/// Увеличение номера последней зарегистрированной команды в БД.
	int increaseLastCommandID();

protected slots:
	/// Обработка выполнения команды на изменения платежа.
	void onPaymentCommandComplete(int aID, EPaymentCommandResult::Enum aError);

	/// Проверяет наличие обновлённых отчётов от модуля обновления.
	void onCheckUpdateReports();

	/// Обновление статусов отложенных команд перезагрузки и выключения.
	void onCheckQueuedRebootCommands();

	/// реальное выполнение команды мониторинга
	/// aEvent реально меет тип PPSDK::EEventType::Enum 
	void doExecuteCommand(int aComandId, int aEvent);

	/// Обработка команды снятия скриншота
	void doScreenshotCommand();

	/// Обработка изменения конфигурации устройств
	void onDeviceConfigurationUpdated();

	/// Уведомление об изменении в папке update
	void onUpdateDirChanged();
	
	/// Обработчик события о закрытии модуля
	void onModuleClosed(const QString & aModuleName);

	/// Обработчик события об изменении состояния устройства
	void onDeviceStatusChanged(const QString & aConfigName);

private:
	/// Проверяет наличие обновлённых отчётов от модуля обновления, возвращает кол-во выполненных команд
	int checkUpdateReports();

	/// Проверка времени жизни команд в очереди
	int checkCommandsLifetime();

	/// Создание новой пары ключей
	static void doGenerateKeyCommand(RemoteService * aMonitoringService, const QString & aLogin, const QString & aPassword);

	/// проверка разрешения на создание команды обновления
	bool allowUpdateCommand();

	/// Запустить команду обновления на выполнение
	int startUpdateCommand(UpdateCommand aCommand);

	/// Проверка разрешения на выключение/перезагрузку
	bool allowRestart();

	/// Проверить команду, не пора ли начинать прошивать железку
	bool checkFirmwareUpload(int aCommandID);

	/// Сохранение списка команд перед выключением
	void saveCommandQueue();

	/// Восстановление списка команд перед запуском
	void restoreCommandQueue();

public:
	/// Найти в списке команду с нужным типом
	UpdateCommand findUpdateCommand(EUpdateType aType);

private:
	/// Обновить список файлов 
	void restartUpdateWatcher(QFileSystemWatcher * aWatcher = nullptr);

	void timerEvent(QTimerEvent * aEvent);

	/// Закончить обработку команды
	void updateCommandFinish(int aCmdID, EStatus aStatus, QVariantMap aParameters = QVariantMap());

	/// Запустить следующую команду обновления из очереди
	void startNextUpdateCommand();

private:
	IApplication * mApplication;

	IHardwareDatabaseUtils * mDatabase;

	QList<PPSDK::IRemoteClient *> mMonitoringClients;

	/// Синхронизация заполнения очереди команд мониторинга.
	QMutex mCommandMutex;

	/// Номер последней принятой команды.
	int mLastCommand;

	/// Платёжные команды, ожидающие выполнения.
	QMap<int, int> mPaymentCommands;

	/// Выполняющиеся команды обновления.
	QMap<int, UpdateCommand> mUpdateCommands;

	/// Выполняющиеся команды снятия скриншотов.
	QList<int> mScreenShotsCommands;

	/// Выполняющаяся команда перегенерации ключей терминала.
	int mGenerateKeyCommand;
	QFuture<void> mGenerateKeyFuture;

	/// Список запланированных команд перезагрузки и выключени.
	QStringList mQueuedRebootCommands;

	/// Настройки сервиса для сохранения очереди выполняющихся команд
	QSettings mSettings;

	QTimer mCheckUpdateReportsTimer;
};

//---------------------------------------------------------------------------

