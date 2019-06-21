/* @file Реализация менеджера сетевых запросов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QThread>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QSharedPointer>
#include <QtCore/QPointer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>

//------------------------------------------------------------------------
namespace CNetworkTaskManager
{
	const char LogName[] = "DownloadManager";
	const int BadTask     = -1;
}

//------------------------------------------------------------------------
class NetworkTask;

//------------------------------------------------------------------------
/// Класс для упрощения отправки сетевых запросов. Поддерживаются блокирующие и
/// неблокирующие механизмы. Все публичные методы потокозащищены.
class NetworkTaskManager : public QThread, public ILogable
{
	Q_OBJECT

public:
	NetworkTaskManager(ILog * aLog = nullptr);
	virtual ~NetworkTaskManager();

	/// Установка proxy сервера.
	void setProxy(const QNetworkProxy & aProxy);

	/// Устанавливает ограничение скорости закачки в процентах от максимальной возможной.
	void setDownloadSpeedLimit(int aPercent);

	/// Добавление нового задания в очередь.
	void addTask(NetworkTask * aTask);

	/// Удаление задания.
	void removeTask(NetworkTask * aTask);

	/// Очищает очередь сетевых запросов с обрывом соединения
	void clearTasks();

	// Установка User-Agent
	void setUserAgent(const QString & aUserAgent);

	// Получение User-Agent
	QString getUserAgent() const;

private slots:
	/// Синхронизированная установка proxy.
	void onSetProxy(QNetworkProxy aProxy);

	/// Синхронизированная установка лимита скорости загрузки.
	void onSetDownloadSpeedLimit(int aPercent);

	/// Синхронизированное добавление нового задания.
	void onAddTask(NetworkTask * aTask);

	/// Синхронизированное удаление задания.
	void onRemoveTask(NetworkTask * aTask);

	/// Синхронизированное уладение всех заданий.
	void onClearTasks();

	/// Обработка сигналов от QNetworkReply.
	void onTaskProgress(qint64 aReceived, qint64 aTotal);
	void onTaskUploadProgress(qint64 aReceived, qint64 aTotal);
	void onTaskReadyRead();
	void onTaskError(QNetworkReply::NetworkError aError);
	void onTaskSslErrors(const QList<QSslError> & aErrors);
	void onTaskComplete();

private:
	/// Рабочая процедура нити.
	void run();

	/// Загрузка сертификата из ресурсов
	QSslCertificate loadCertResource(const QString & aPath);

signals:
	/// Сигнал об неуспешном сетевом соединении
	void networkTaskStatus(bool aFailure);

private:
	typedef QMap<QNetworkReply *, QPointer<NetworkTask>> TTaskMap;

	TTaskMap mTasks;
	QSharedPointer<QNetworkAccessManager> mNetwork;
	QString mUserAgent;
	void loadCerts();
};

//------------------------------------------------------------------------
