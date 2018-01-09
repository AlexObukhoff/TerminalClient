#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtNetwork/QLocalSocket>
#include <Common/QtHeadersEnd.h>

//----------------------------------------------------------------------------
namespace CIMessageQueueClient
{
	const QString DefaultLog = "MessageQueueClient";
	const int ConnectionTimeout = 1000; // msecs

	enum ErrorCode
	{
		ConnectionRefusedError = QLocalSocket::ConnectionRefusedError,
		PeerClosedError = QLocalSocket::PeerClosedError,
		ServerNotFoundError = QLocalSocket::ServerNotFoundError,
		SocketAccessError = QLocalSocket::SocketAccessError,
		SocketResourceError = QLocalSocket::SocketResourceError,
		SocketTimeoutError = QLocalSocket::SocketTimeoutError,
		DatagramTooLargeError = QLocalSocket::DatagramTooLargeError,
		ConnectionError = QLocalSocket::ConnectionError,
		UnsupportedSocketOperationError = QLocalSocket::UnsupportedSocketOperationError,
		UnknownSocketError = QLocalSocket::UnknownSocketError
	};
}

//----------------------------------------------------------------------------
class QObject;

//----------------------------------------------------------------------------
class IMessageQueueClient
{
public:
	virtual ~IMessageQueueClient() {};
	
	/// Подключиться к очереди сообщений aQueueName.
	virtual bool connect(const QString & aQueueName) = 0;
	/// Отключиться от текущей очереди сообщений.
	virtual void disconnect() = 0;
	/// Возвращает статус подключения.
	virtual bool isConnected() const = 0;
	
	/// Послать сообщение серверу.
	virtual void sendMessage(const QByteArray & aMessage) = 0;
	
	/// Подписаться на получение сообщения. aObject должен иметь 
	/// слот onMessageReceived(QByteArray aMessage).
	virtual bool subscribeOnMessageReceived(QObject * aObject) = 0;

	/// Подписаться на получение сообщения. aObject должен иметь 
	/// слот onDisconnected().
	virtual bool subscribeOnDisconnected(QObject * aObject) = 0;

	/// Подписывает aObject на все сообщения. aObject должен иметь
	/// слоты onMessageReceived(QByteArray aMessage), onDisconnected(),
	/// onError(CIMessageQueueClient::ErrorCode aErrorCode, const QString & aErrorMessage).
	virtual bool subscribeOnEvents(QObject * aObject) = 0;
};

//----------------------------------------------------------------------------
IMessageQueueClient* createMessageQueueClient();

//----------------------------------------------------------------------------
