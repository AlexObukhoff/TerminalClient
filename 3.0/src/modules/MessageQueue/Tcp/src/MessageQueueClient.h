#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QTimer>
#include <QtNetwork/QTcpSocket>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>

// Project
#include "MessageQueue/IMessageQueueClient.h"

//----------------------------------------------------------------------------
class MessageQueueClient : public QObject,
                           public IMessageQueueClient,
                           public ILogable
{
	Q_OBJECT

public:
	MessageQueueClient();
	virtual ~MessageQueueClient();

	/// Подключиться к очереди сообщений aQueueName.
	virtual bool connect(const QString& aQueueName);
	/// Отключиться от текущей очереди сообщений.
	virtual void disconnect();
	/// Возвращает статус подключения.
	virtual bool isConnected() const;
	
	/// Послать сообщение серверу.
	virtual void sendMessage(const QByteArray& aMessage);
	
	/// Подписаться на получение сообщения. aObject должен иметь 
	/// слот onMessageReceived(QByteArray aMessage).
	virtual bool subscribeOnMessageReceived(QObject* aObject);

	/// Подписаться на получение сообщения. aObject должен иметь 
	/// слот onDisconnected().
	virtual bool subscribeOnDisconnected(QObject * aObject);

	/// Подписывает aObject на все сообщения. aObject должен иметь
	/// слоты onMessageReceived(QByteArray aMessage), onDisconnected(),
	/// onError(CIMessageQueueClient::ErrorCode aErrorCode, const QString & aErrorMessage).
	virtual bool subscribeOnEvents(QObject* aObject);

private:
	void parseInputBuffer(QByteArray &aBuffer);

private slots:
	void onSocketReadyRead();
	void onSocketError(QAbstractSocket::SocketError aErrorCode);
	void onSocketDisconnected();
	void pingServer();

signals:
	void onMessageReceived(QByteArray aMessage);
	void onError(CIMessageQueueClient::ErrorCode aErrorCode, const QString & aErrorMessage);
	void onDisconnected();

private:
	QTcpSocket m_socket;
	QByteArray m_buffer;

	/// Таймер, который будет следить за ответом сервера на пинг.
	QTimer m_answerTimer;
};

//----------------------------------------------------------------------------
