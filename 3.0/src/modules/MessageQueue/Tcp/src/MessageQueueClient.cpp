#include "MessageQueueClient.h"
#include "MessageQueue/MessageQueueConstants.h"

//----------------------------------------------------------------------------
MessageQueueClient::MessageQueueClient()
	: ILogable(CIMessageQueueClient::DefaultLog)
{
	QObject::connect(&m_socket, SIGNAL(readyRead()), 
		this, SLOT(onSocketReadyRead()));
	QObject::connect(&m_socket, SIGNAL(error(QAbstractSocket::SocketError)), 
		this, SLOT(onSocketError(QAbstractSocket::SocketError)));
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
	m_socket.connectToHost("127.0.0.1", aQueueName.toUShort());
	m_socket.waitForConnected(CIMessageQueueClient::ConnectionTimeout);

	return isConnected();
}

//----------------------------------------------------------------------------
void MessageQueueClient::disconnect()
{
	m_socket.disconnectFromHost();
}

//----------------------------------------------------------------------------
bool MessageQueueClient::isConnected() const
{
	return (m_socket.state() == QAbstractSocket::ConnectedState);
}

//----------------------------------------------------------------------------
void MessageQueueClient::sendMessage(const QByteArray& aMessage)
{
	if (m_socket.state() == QTcpSocket::ConnectedState)
	{
		m_socket.write(aMessage + '\0');
		m_socket.flush();
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
	while (m_socket.bytesAvailable() > 0)
	{
		QByteArray buffer =  m_socket.readAll();

		toLog(LogLevel::Normal, QString::fromUtf8(buffer));

		m_buffer +=  buffer;
		parseInputBuffer(m_buffer);
	}
}

//----------------------------------------------------------------------------
void MessageQueueClient::onSocketError(QAbstractSocket::SocketError aErrorCode)
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
