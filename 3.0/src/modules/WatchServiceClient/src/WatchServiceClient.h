/* @file Реализация клиента сторожевого сервиса. */

#pragma once

// Boost
#include <boost/function.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QTimer>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QStringList>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// Modules
#include <MessageQueue/IMessageQueueClient.h>
#include <WatchServiceClient/IWatchServiceClient.h>

//---------------------------------------------------------------------------
class WatchServiceClient : public QThread,
                           public IWatchServiceClient
{
	Q_OBJECT

public:
	WatchServiceClient(const QString & aName, PingThread aThread);
	virtual ~WatchServiceClient();

	/// IWatchServiceClient: Подключение к сторожевому сервису.
	virtual bool start();

	/// IWatchServiceClient: Закрытие соединения со сторожевым сервисом.
	virtual void stop();

	/// IWatchServiceClient: Возвращает true, если клиент подключён к сторожевому сервису.
	virtual bool isConnected() const;

	/// IWatchServiceClient: Выполнение команды aCommand на модуле aModule c параметрами aParams.
	virtual void execute(QString aCommand, QString aModule = QString(), QString aParams = QString());

	/// IWatchServiceClient: Завршение работы всех модулей и сторожевого сервиса.
	virtual void stopService();

	/// IWatchServiceClient: Команда перезапуска сервиса.
	virtual void restartService(QStringList aParameters);
	
	/// IWatchServiceClient: Перезагрузка компьютера.
	virtual void rebootMachine();

	/// IWatchServiceClient: Выключение компьютера.
	virtual void shutdownMachine();

	/// IWatchServiceClient: Запуск модуля aModule с параметрами aParams.
	virtual void startModule(QString aModule, QString aParams = QString());
	
	/// IWatchServiceClient: Закрытие модуля aModule.
	virtual void closeModule(QString aModule);

	/// Закрытие всех активных модулей без закрытия сервиса.
	virtual void closeModules();

	/// IWatchServiceClient: Включение защитного экрана.
	virtual void showSplashScreen();

	/// IWatchServiceClient: Отключение защитного экрана.
	virtual void hideSplashScreen();

	/// IWatchServiceClient: Сообщить сервису о статусе aStatus состояния aType.
	virtual void setState(int aType, int aStatus);

	/// IWatchServiceClient: Сбрасывает все установленные статусы.
	virtual void resetState();

	/// IWatchServiceClient: Подписывает aObject на получение команд от сервиса. Объект должен иметь
	/// слот с сигнатурой onCommandReceived(const QString& aSender, const QString& aTarget, 
	/// const QString& aType, const QStringList & aTail).
	virtual bool subscribeOnCommandReceived(QObject * aObject);

	/// IWatchServiceClient: Подписывает aObject на получение оповещения от сервиса о требованием завершить работу.
	/// Объект должен иметь слот с сигнатурой onCloseCommandReceived().
	virtual bool subscribeOnCloseCommandReceived(QObject * aObject);

	/// IWatchServiceClient: Подписывает aObject на получение оповещения о разрыве связи со сторожевым сервисом.
	/// Объект должен иметь слот с сигнатурой onDisconnected().
	virtual bool subscribeOnDisconnected(QObject * aObject);

	/// Подписывает aObject на получение оповещения от сервиса о закрытии любого модуля.
	/// Объект должен иметь слот с сигнатурой onModuleClosed(const QString &).
	virtual bool subscribeOnModuleClosed(QObject * aObject);

protected:
	/// Рабочая процедура Qt'шной нитки.
	virtual void run();

	/// Пинг сторожевого сервиса.
	virtual void ping();

private:
	typedef boost::function<void()> TMethod;

	// Отправка сообщения по транспортному каналу.
	void sendMessage(const QByteArray & aMessage);

signals:
	/// Вызов указанного метода в своём потоке.
	void invokeMethod(WatchServiceClient::TMethod aMethod);

	/// Клиент отключился от сторожевого сервиса.
	void disconnected();

	/// От сервера получена команда на завершение работы.
	void onCloseCommandReceived();

	/// Сигнал, сообщающий о закрытии модуля
	void onModuleClosed(const QString & aModuleName);

	/// От севера получена команда.
	void onCommandReceived(const QString& aSender, const QString& aTarget, const QString& aType, const QStringList & aTail);

protected slots:
	/// Обработчик при вызове метода из родного потока.
	void onInvokeMethod(WatchServiceClient::TMethod aMethod);

	/// Обработчик для пинга сторожевого сервиса.
	void onPing();

	/// Обработчик получаемых от сервера данных.
	void onMessageReceived(QByteArray aMessage);

	/// Получаем сообщение об отклчюении с транспортного уровня.
	void onDisconnected();

private:
	QSharedPointer<IMessageQueueClient> mClient;
	
	QTimer mPingTimer;
	
	QString mName;

	QWaitCondition mInitCondition;
	QMutex mInitMutex;
};

//---------------------------------------------------------------------------
