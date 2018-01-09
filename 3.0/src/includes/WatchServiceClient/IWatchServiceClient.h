#pragma once

// Qt
#include "Common/QtHeadersBegin.h"
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include "Common/QtHeadersEnd.h"

// Modules
#include "MessageQueue/IMessageQueueClient.h"

class QObject;

//---------------------------------------------------------------------------
class IWatchServiceClient
{
public:
	typedef enum
	{
		DedicateThread,
		MainThread
	} PingThread;

public:
	virtual ~IWatchServiceClient() {};

	/// Подключение к сторожевому сервису.
	virtual bool start() = 0;

	/// Закрытие соединения со сторожевым сервисом.
	virtual void stop() = 0;

	/// Возвращает true, если клиент подключён к сторожевому сервису.
	virtual bool isConnected() const = 0;

	/// Выполнение команды aCommand на модуле aModule c параметрами aParams.
	virtual void execute(QString aCommand, QString aModule = QString(), QString aParams = QString()) = 0;

	/// Завершение работы всех модулей и сторожевого сервиса.
	virtual void stopService() = 0;

	/// Команда перезапуска сервиса.
	virtual void restartService(QStringList aParameters) = 0;
	
	/// Перезагрузка компьютера.
	virtual void rebootMachine() = 0;

	/// Выключение компьютера.
	virtual void shutdownMachine() = 0;

	/// Запуск модуля aModule с параметрами aParams.
	virtual void startModule(QString aModule, QString aParams = QString()) = 0;
	
	/// Закрытие модуля aModule.
	virtual void closeModule(QString aModule) = 0;

	/// Закрытие всех активных модулей без закрытия сервиса.
	virtual void closeModules() = 0;

	/// Включение защитного экрана.
	virtual void showSplashScreen() = 0;

	/// Отключение защитного экрана.
	virtual void hideSplashScreen() = 0;

	/// Сообщить сервису о статусе aStatus состояния aType.
	virtual void setState(int aType, int aStatus) = 0;

	/// Сбрасывает все установленные статусы.
	virtual void resetState() = 0;

	/// Подписывает aObject на получение команд от сервиса. Объект должен иметь
	/// слот с сигнатурой onCommandReceived(const QString& aSender, const QString& aTarget, 
	/// const QString& aType, const QStringList & aTail).
	virtual bool subscribeOnCommandReceived(QObject * aObject) = 0;

	/// Подписывает aObject на получение оповещения от сервиса о требованием завершить работу.
	/// Объект должен иметь слот с сигнатурой onCloseCommandReceived().
	virtual bool subscribeOnCloseCommandReceived(QObject * aObject) = 0;

	/// Подписывает aObject на получение оповещения о разрыве связи со сторожевым сервисом.
	/// Объект должен иметь слот с сигнатурой onDisconnected().
	virtual bool subscribeOnDisconnected(QObject * aObject) = 0;

	/// Подписывает aObject на получение оповещения от сервиса о закрытии любого модуля.
	/// Объект должен иметь слот с сигнатурой onModuleClosed(const QString &).
	virtual bool subscribeOnModuleClosed(QObject * aObject) = 0;
};

//---------------------------------------------------------------------------
IWatchServiceClient * createWatchServiceClient(const QString & aClientName, IWatchServiceClient::PingThread aThread = IWatchServiceClient::DedicateThread);

//---------------------------------------------------------------------------
