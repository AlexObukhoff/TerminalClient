#include "MessageQueueServer.h"
#include "MessageQueue/MessageQueueConstants.h"

//----------------------------------------------------------------------------
MessageQueueServer::MessageQueueServer(const QString& aQueueName)
{
	m_log = ILog::getInstance(CIMessageQueueServer::DefaultLog);

	m_queueName = aQueueName;
}

//----------------------------------------------------------------------------
MessageQueueServer::MessageQueueServer(const QString& aQueueName, ILog* aLog)
{
	m_log = aLog;

	m_queueName = aQueueName;
}

//----------------------------------------------------------------------------
MessageQueueServer::~MessageQueueServer()
{
}

//----------------------------------------------------------------------------
bool MessageQueueServer::init()
{
	if (!connect(&m_disconnectSignalMapper, SIGNAL(mapped(QObject*)), this, SLOT(onSocketDisconnected(QObject*))) ||
		!connect(&m_readyReadSignalMapper, SIGNAL(mapped(QObject*)), this, SLOT(onSocketReadyRead(QObject*))))
		return false;

	if (!isListening())
		return listen(m_queueName);

	return true;
}

//----------------------------------------------------------------------------
void MessageQueueServer::stop()
{
	QLocalServer::close();
}

//----------------------------------------------------------------------------
bool MessageQueueServer::subscribeOnMessageReceived(QObject* aObject)
{
	return connect(this, SIGNAL(onMessageReceived(QByteArray)), aObject, SLOT(onMessageReceived(QByteArray)));
}

//----------------------------------------------------------------------------
bool MessageQueueServer::subscribeOnDisconnected(QObject * aObject)
{
	return connect(this, SIGNAL(onDisconnected()), aObject, SLOT(onDisconnected()));
}

//----------------------------------------------------------------------------
void MessageQueueServer::sendMessage(const QByteArray &aMessage)
{
	foreach (QLocalSocket* socket, m_sockets.keys())
	{
		if (socket->state() == QLocalSocket::ConnectedState)
		{
			socket->write(aMessage + '\0');
			socket->flush();
			socket->waitForBytesWritten(100);
		}
	}
}

//----------------------------------------------------------------------------
void MessageQueueServer::incomingConnection(quintptr aSocketDescriptor)
{
	LOG(m_log,
		LogLevel::Normal,
		QString("New incoming connection... Socket with descriptor %1 has been connected.")
		.arg(aSocketDescriptor));

	QLocalSocket* newSocket = new QLocalSocket(this);
	newSocket->setSocketDescriptor(aSocketDescriptor);

	m_disconnectSignalMapper.setMapping(newSocket, newSocket);
	connect(newSocket, SIGNAL(disconnected()), &m_disconnectSignalMapper, SLOT(map()));

	m_readyReadSignalMapper.setMapping(newSocket, newSocket);
	connect(newSocket, SIGNAL(readyRead()), &m_readyReadSignalMapper, SLOT(map()));

	m_sockets[newSocket] = aSocketDescriptor;
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketDisconnected(QObject* aObject)
{
	emit onDisconnected();

	QLocalSocket* socket = dynamic_cast<QLocalSocket*>(aObject);

	if (socket)
	{
		LOG(m_log, LogLevel::Normal, QString("Socket with descriptor %1 has been disconnected.")
			.arg(m_sockets[socket]));

		m_buffers.remove(m_sockets[socket]);
		m_sockets.remove(socket);
	}

	aObject->deleteLater();
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketReadyRead(QObject *aObject)
{
	QLocalSocket* socket = dynamic_cast<QLocalSocket*>(aObject);
	if (!socket)
	{
		LOG(m_log, LogLevel::Error, "Wrong object was passed to onSocketReadyRead slot...");
		return;
	}

	QByteArray newData = socket->readAll();
	
	/*if (newData.indexOf(MessageQueueConstants::PingMessage) >= 0)
	{
		sendMessage(MessageQueueConstants::PingMessage);
		return;
	}*/

	quintptr socketDescriptor = socket->socketDescriptor();

	if (m_buffers.contains(socketDescriptor))
	{
		m_buffers[socketDescriptor] = m_buffers[socketDescriptor] + newData;
	}
	else
	{
		m_buffers[socketDescriptor] = newData;
	}

	parseInputBuffer(m_buffers[socketDescriptor]);
}

//----------------------------------------------------------------------------
void MessageQueueServer::parseInputBuffer(QByteArray &aBuffer)
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
