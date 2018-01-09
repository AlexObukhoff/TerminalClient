#include "MessageQueueClient.h"
#include "MessageQueue/MessageQueueConstants.h"

//----------------------------------------------------------------------------
MessageQueueClient::MessageQueueClient()
{
	QObject::connect(&m_socket, SIGNAL(readyRead()), 
		this, SLOT(onSocketReadyRead()));
	QObject::connect(&m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), 
		this, SLOT(onSocketError(QLocalSocket::LocalSocketError)));
	QObject::connect(&m_socket, SIGNAL(disconnected()),
		this, SLOT(onSocketDisconnected()));
	//QObject::connect(&m_answerTimer, SIGNAL(timeout()), this, SLOT(onSocketDisconnected()));	
}

//----------------------------------------------------------------------------
MessageQueueClient::~MessageQueueClient()
{

}

//----------------------------------------------------------------------------
bool MessageQueueClient::connect(const QString& aQueueName)
{
	m_socket.connectToServer(aQueueName);
	m_socket.waitForConnected(CIMessageQueueClient::ConnectionTimeout);

	return m_socket.isOpen();
}

//----------------------------------------------------------------------------
void MessageQueueClient::disconnect()
{
	m_socket.disconnectFromServer();
}

//----------------------------------------------------------------------------
bool MessageQueueClient::isConnected() const
{
	return m_socket.isOpen();
}

//----------------------------------------------------------------------------
void MessageQueueClient::sendMessage(const QByteArray& aMessage)
{
	if (m_socket.state() == QLocalSocket::ConnectedState)
	{
		m_socket.write(aMessage + '\0');
		m_socket.flush();
		m_socket.waitForBytesWritten(100);
	}
}

//----------------------------------------------------------------------------
bool MessageQueueClient::subscribeOnMessageReceived(QObject* aObject)
{
	return QObject::connect(
		this, SIGNAL(onMessageReceived(QByteArray)),
		aObject, SLOT(onMessageReceived(QByteArray))
		);	
}

//----------------------------------------------------------------------------
bool MessageQueueClient::subscribeOnDisconnected(QObject* aObject)
{
	return QObject::connect(this, SIGNAL(onDisconnected()),
		aObject, SLOT(onDisconnected()));
}

//----------------------------------------------------------------------------
bool MessageQueueClient::subscribeOnEvents(QObject* aObject)
{
	//QTimer::singleShot(MessageQueueConstants::PingTime, this, SLOT(pingServer()));
	return QObject::connect(
		this, SIGNAL(onMessageReceived(QByteArray)),
		aObject, SLOT(onMessageReceived(QByteArray))
		) && QObject::connect(
		this, SIGNAL(onError(CIMessageQueueClient::ErrorCode, const QString &)),
		aObject, SLOT(onError(CIMessageQueueClient::ErrorCode, const QString &))
		) && QObject::connect(
		this, SIGNAL(onDisconnected()),
		aObject, SLOT(onDisconnected()));
}

//----------------------------------------------------------------------------
void MessageQueueClient::onSocketReadyRead()
{
	QByteArray buffer =  m_socket.readAll();
	/*if (buffer.indexOf(MessageQueueConstants::PingMessage) >= 0)
	{
		m_answerTimer.stop();
		QTimer::singleShot(MessageQueueConstants::PingTime, this, SLOT(pingServer()));
		return;
	}*/
	m_buffer +=  buffer;
	parseInputBuffer(m_buffer);
}

//----------------------------------------------------------------------------
void MessageQueueClient::onSocketError(QLocalSocket::LocalSocketError aErrorCode)
{
	emit onError(static_cast<CIMessageQueueClient::ErrorCode>(aErrorCode), m_socket.errorString());
}

//----------------------------------------------------------------------------
void MessageQueueClient::onSocketDisconnected()
{
	emit onDisconnected();
}

//----------------------------------------------------------------------------
void MessageQueueClient::parseInputBuffer(QByteArray &aBuffer)
{
	int messageEnd = aBuffer.indexOf('\0');
	while (messageEnd != -1)
	{
		QByteArray newMessageData = aBuffer.left(messageEnd);		

		aBuffer = aBuffer.right(aBuffer.size() - messageEnd - 1);

		emit onMessageReceived(newMessageData);

		messageEnd = aBuffer.indexOf('\0');
	}
}

//----------------------------------------------------------------------------
void MessageQueueClient::pingServer()
{
	sendMessage(MessageQueueConstants::PingMessage);
	m_answerTimer.start(MessageQueueConstants::AnswerFromServerTime);
}

//----------------------------------------------------------------------------
