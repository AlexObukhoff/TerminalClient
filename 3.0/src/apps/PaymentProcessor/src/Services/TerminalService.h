/* @file Реализация сервиса управления терминалом. */

#pragma once

// boost
#include <boost/optional.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include <QtCore/QMultiMap>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/ITerminalService.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/Drivers/WarningLevel.h>

#include <WatchServiceClient/IWatchServiceClient.h>

// Project
#include "TerminalStatusDescriptions.h"


class IHardwareDatabaseUtils;
class IApplication;
class GUIService;

//---------------------------------------------------------------------------
class TerminalService : public QObject, public SDK::PaymentProcessor::IService, protected ILogable, public SDK::PaymentProcessor::ITerminalService
{
	Q_OBJECT

	/// Интервал между попытками восстановления конфигов в минутах.
	static const int ConfigRestoreInterval = 30;

public:
	static TerminalService * instance(IApplication * aApplication);

	TerminalService(IApplication * aApplication);
	virtual ~TerminalService();

	/// IService: инициализация сервиса.
	virtual bool initialize();

	/// IService: Закончена инициализация всех сервисов.
	virtual void finishInitialize();

	/// IService: Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	/// IService: завершение работы сервиса.
	virtual bool shutdown();

	/// IService: возвращает имя сервиса.
	virtual QString getName() const;

	/// IService: список зависимостей.
	virtual const QSet<QString> & getRequiredServices() const;

	/// Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);

#pragma region ITerminalService
	/// ITerminalService: наличие блокировки.
	virtual bool isLocked() const;

	/// ITerminalService: установка блокировки.
	virtual void setLock(bool aIsLocked);

	/// ITerminalService: скачать конфиги при следующей загрузке.
	virtual void needUpdateConfigs();

	/// Снять/поставить признак ошибки терминала
	virtual void setTerminalError(SDK::PaymentProcessor::ETerminalError::Enum aErrorType, bool aError);

	/// Проверка наличия ошибки терминала
	virtual bool isTerminalError(SDK::PaymentProcessor::ETerminalError::Enum aErrorType) const;

	/// Отправить сообщение разработчикам
	virtual void sendFeedback(const QString & aSenderSubsystem, const QString & aMessage);
#pragma endregion

	/// Получить список устройств с ошибкой.
	QMap<QString, int> getFaultyDevices(bool aActual = false) const;

	/// Получить WS-клиент.
	IWatchServiceClient * getClient();

	/// Получить имена всех устройств.
	QStringList getDeviceNames() const;

	/// Получить имена всех устройств - валидаторов валюты.
	QStringList getAcceptorTypes() const;

private:
	/// Заблокирован ли?
	bool isDisabled() const;

	/// Запись статус блокировки для устройства "терминал" в БД.
	void writeLockStatus(bool aIsLocked);

	/// Изменение статуса и времени обновления счетчика числа перезапусков в день.
	int getRestartCount() const;
	void setRestartCount(int aCount);

	/// Возвращает обобщённый статус устройства.
	QPair<SDK::Driver::EWarningLevel::Enum, QString> getTerminalStatus() const;

	/// Отключаем/включаем графический интерфейс.
	void updateGUI();

	/// Обновить статус терминала
	void updateTerminalStatus();


private slots:
	void onEvent(const SDK::PaymentProcessor::Event & aEvent);

	/// Обновление статуса устройства.
	void onDeviceStatusChanged(const QString & aConfigName, SDK::Driver::EWarningLevel::Enum aLevel, const QString & aDescription, int aStatus);

	/// Реакция на обновление конфигурации.
	void onHardwareConfigUpdated();

	/// Проверка целостности конфигурационных файлов. Отправляет команду загрузки конфигов на мониторинг в случае её нарушения.
	void checkConfigsIntegrity();

private:
	IApplication  * mApplication;
	SDK::PaymentProcessor::IEventService * mEventService;
	IHardwareDatabaseUtils * mDbUtils;

	/// Содержит имя конфигурации устройства, находящегося в состоянии ошибки.
	QMultiMap<QString, int> mDeviceErrorFlags;

	SDK::PaymentProcessor::SCommonSettings mSettings;

	QSharedPointer<IWatchServiceClient> mClient;

	/// Кэш значения заблокированности терминала (т.к. флаг блокировки должен сохраняться, даже если БД сломана)
	mutable boost::optional<bool> mLocked;

	/// Кэш статуса терминала
	QMap<QString, SDK::PaymentProcessor::Event> mTerminalStatusHash;
	QPair<SDK::Driver::EWarningLevel::Enum, QString> mTerminalStatusCache;
};

//---------------------------------------------------------------------------

