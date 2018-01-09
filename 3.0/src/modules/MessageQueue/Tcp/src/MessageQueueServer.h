#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QSignalMapper>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILog.h>

// Project
#include "MessageQueue/IMessageQueueServer.h"

//----------------------------------------------------------------------------
class MessageQueueServer : public QTcpServer,
                           public IMessageQueueServer
{
	typedef QMap<QTcpSocket*, quintptr> TLocalSocketMap;
	typedef QMap<quintptr, QByteArray> TSocketBufferMap;

	Q_OBJECT

public:
	MessageQueueServer(const QString& aName);
	MessageQueueServer(const QString& aName, ILog* aLog);
	virtual ~MessageQueueServer();

	/// Активировать очередь сообщений.
	virtual bool init();
	/// Останавливает сервер.
	virtual void stop();
	
	/// Послать сообщение всем подключенным клиентам.
	virtual void sendMessage(const QByteArray& aMessage);
	
	/// Подписаться на получение сообщения. aObject должен иметь 
	/// слот onMessageReceived(QByteArray aMessage).
	virtual bool subscribeOnMessageReceived(QObject* aObject);

	/// Подписаться на получение сообщения. aObject должен иметь 
	/// слот onDisconnected().
	virtual bool subscribeOnDisconnected(QObject * aObject);

protected:
	virtual void incomingConnection(int socketDescriptor);

private:
	QByteArray parseInputBuffer(QByteArray aBuffer);

signals:
	void onMessageReceived(QByteArray aMessage);
	void onDisconnected();

private slots:
	void onSocketDisconnected(QObject* aObject);
	void onSocketReadyRead(QObject* aObject);

private:
	QSignalMapper m_disconnectSignalMapper;
	QSignalMapper m_readyReadSignalMapper;
	TLocalSocketMap m_sockets;
	TSocketBufferMap m_buffers;
	QString m_queueName;
	ILog* m_log;
};

//----------------------------------------------------------------------------
