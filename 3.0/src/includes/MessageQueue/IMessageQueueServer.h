#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

//----------------------------------------------------------------------------
namespace CIMessageQueueServer
{
	const QString DefaultLog = "MessageQueue";
	const QString DefaultName = "CyberPlatMessageQueue";
}

//----------------------------------------------------------------------------
class IMessageQueueServer
{
public:
	virtual ~IMessageQueueServer() {};

	/// Активировать очередь сообщений.
	virtual bool init() = 0;
	/// Останавливает сервер.
	virtual void stop() = 0;
	
	/// Послать сообщение всем подключенным клиентам.
	virtual void sendMessage(const QByteArray& aMessage) = 0;
	
	/// Подписаться на получение сообщения. aObject должен иметь 
	/// слот onMessageReceived(QByteArray aMessage).
	virtual bool subscribeOnMessageReceived(QObject* aObject) = 0;

	/// Подписаться на получение сообщения. aObject должен иметь 
	/// слот onDisconnected().
	virtual bool subscribeOnDisconnected(QObject * aObject) = 0;
};

//----------------------------------------------------------------------------
IMessageQueueServer* createMessageQueueServer(const QString& aQueueName);
IMessageQueueServer* createMessageQueueServer(const QString& aQueueName, ILog* aLog);

//----------------------------------------------------------------------------
