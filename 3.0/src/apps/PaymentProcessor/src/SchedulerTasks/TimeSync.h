/* @file Реализация задачи синхронизации системного времени терминала. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDateTime>
#include <QtCore/QUrl>
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtNetwork/QHostAddress>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ITask.h>

// Модули
#include <Common/ILogable.h>

class NetworkTaskManager;
class NtpClient;
class NtpReply;

//---------------------------------------------------------------------------
class TimeSync : public QObject, public SDK::PaymentProcessor::ITask, private ILogable
{
	Q_OBJECT

	NetworkTaskManager * mNetwork;

	NtpClient * mClient;        /// NTP клиент
	QSet<QUrl> mTimeSyncHosts; /// Список NTP серверов и http серверов для приблизительной синхронизации времени
	bool mTimeReceived;         /// Признак, что время синхронизированно
	bool mCanceled;

public:
	TimeSync(const QString & aName, const QString & aLogName, const QString & aParams);

	/// выполнить задачу
	virtual void execute();

	/// остановить выполнение задачи
	virtual bool cancel();

	/// подписаться на сигнал окончания задания
	virtual bool subscribeOnComplete(QObject * aReceiver, const char * aSlot);

private slots:
	void ntpReplyReceived(const QHostAddress & aAddress, quint16 aPort, const NtpReply & aReply);
	void httpReplyReceived();

private:
	void checkNextUrl();

	void timeReceived(QDateTime aServerDateTime);
	void timeOffsetReceived(qint64 aLocalTimeOffset);

	void ntpCheckMethod(const QUrl & aHost);
	
	/// Приблизительная проверка времени
	void httpCheckMethod(const QUrl & aHost);

private slots:
	/// Обрабатывает таймаут запросов к серверам NTP
	void ntpRequestTimeout();

signals:
	void finished(const QString & aName, bool aComplete);
};


