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
		return listen(QHostAddress::LocalHost, static_cast<qint16>(m_queueName.toInt()));

	return true;
}

//----------------------------------------------------------------------------
void MessageQueueServer::stop()
{
	QTcpServer::close();
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
	foreach (QTcpSocket * socket, m_sockets.keys())
	{
		if (socket->state() == QTcpSocket::ConnectedState)
		{
			socket->write(aMessage + '\0');
			socket->flush();
		}
	}
}

//----------------------------------------------------------------------------
void MessageQueueServer::incomingConnection(int aSocketDescriptor)
{
	LOG(m_log,
		LogLevel::Normal,
		QString("New incoming connection... Socket with descriptor %1 has been connected.")
		.arg(aSocketDescriptor));

	QTcpSocket* newSocket = new QTcpSocket(this);
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

	QTcpSocket* socket = dynamic_cast<QTcpSocket*>(aObject);

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
	QTcpSocket* socket = dynamic_cast<QTcpSocket*>(aObject);
	if (!socket)
	{
		LOG(m_log, LogLevel::Error, "Wrong object was passed to onSocketReadyRead slot...");
		return;
	}

	while (socket->bytesAvailable() > 0)
	{
		QByteArray newData = socket->readAll();
		
		quintptr socketDescriptor = socket->socketDescriptor();

		m_buffers[socketDescriptor] = parseInputBuffer(m_buffers[socketDescriptor].append(newData));
	}
}

//----------------------------------------------------------------------------
QByteArray MessageQueueServer::parseInputBuffer(QByteArray aBuffer)
{
	int messageEnd = aBuffer.indexOf('\0');
	while (messageEnd != -1)
	{
		QByteArray newMessageData = aBuffer.left(messageEnd);

		aBuffer = aBuffer.right(aBuffer.size() - messageEnd - 1);

		emit onMessageReceived(newMessageData);

		messageEnd = aBuffer.indexOf('\0');
	}

	return aBuffer;
}

//----------------------------------------------------------------------------
