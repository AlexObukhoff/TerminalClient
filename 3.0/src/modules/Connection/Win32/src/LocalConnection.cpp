/* @file Реализация соединения по локальной сети. */

// Qt headers
#include "Common/QtHeadersBegin.h"
#include <QtNetwork/QNetworkInterface>
#include "Common/QtHeadersEnd.h"

// Project headers
#include "LocalConnection.h"

//--------------------------------------------------------------------------------
LocalConnection::LocalConnection(const QString & aName, NetworkTaskManager * aNetwork, ILog * aLog)
	: ConnectionBase(aName, aNetwork, aLog)
{
}

//--------------------------------------------------------------------------------
LocalConnection::~LocalConnection()
{
	try
	{
		close();
	}
	catch (...)
	{
		toLog(LogLevel::Fatal, "LocalConnection: An exception occurred while close network connection.");
	}
}

//--------------------------------------------------------------------------------
EConnectionTypes::Enum LocalConnection::getType() const
{
	return EConnectionTypes::Unmanaged;
}

//--------------------------------------------------------------------------------
void LocalConnection::doConnect() throw(...)
{
	// UNDONE
	//toLog(LogLevel::Warning, "Bringing up a local connection is not implemented.");
}

//--------------------------------------------------------------------------------
void LocalConnection::doDisconnect() throw(...)
{
	// UNDONE
	//toLog(LogLevel::Warning, "Shutting down a local connection is not implemented.");
}

//--------------------------------------------------------------------------------
bool LocalConnection::doIsConnected() throw(...)
{
	// FIX: имя локального соединения может быть указано в конфиге неправильно
	//QNetworkInterface intf = QNetworkInterface::interfaceFromName(getName());

	//return intf.isValid() ? (intf.flags() & QNetworkInterface::IsUp) : false;

	return mConnected;
}

//--------------------------------------------------------------------------------
bool LocalConnection::doCheckConnection(const IConnection::CheckUrl & aHost)
{
	return httpCheckMethod(aHost);
}

//----------------------------------------------------------------------------
