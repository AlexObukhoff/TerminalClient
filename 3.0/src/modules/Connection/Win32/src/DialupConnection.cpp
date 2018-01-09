/* @file Реализация dialup-соединения. */

// Модули
#include <Connection/NetworkError.h>

// Проект
#include "RasWrapper.h"
#include "DialupConnection.h"

//--------------------------------------------------------------------------------
DialupConnection::DialupConnection(const QString & aName, NetworkTaskManager * aNetwork, ILog * aLog)
	: ConnectionBase(aName, aNetwork, aLog)
{
}

//--------------------------------------------------------------------------------
DialupConnection::~DialupConnection()
{
	try
	{
		close();
	}
	catch (...)
	{
		toLog(LogLevel::Fatal, "DialupConnection: An exception occurred while close network connection.");
	}
}

//--------------------------------------------------------------------------------
EConnectionTypes::Enum DialupConnection::getType() const
{
	return EConnectionTypes::Dialup;
}

//--------------------------------------------------------------------------------
void DialupConnection::doConnect() throw(...)
{
	RasApi::PhonebookEntryName entryName;
	entryName.setName(mName.toStdWString());

	RasApi::PhonebookEntry entry;
	if (RasApi::GetEntryProperties(entryName, entry) == 0)
	{
		toLog(LogLevel::Normal, QString("Device name:    %1 (%2)")
			.arg(QString::fromStdWString(entry.deviceName()))
			.arg(QString::fromStdWString(RasApi::getAttachedTo(entry.deviceName()))));
		toLog(LogLevel::Normal, QString("Phone number:   %1").arg(QString::fromStdWString(entry.localPhoneNumber())));
		toLog(LogLevel::Normal, QString("*").repeated(40));
	}

	//TODO: probably need to add win2000 phonebook path workaround

	DWORD raserror = RasApi::Dial(entryName);

	if (raserror == RPC_S_SERVER_UNAVAILABLE || raserror == RPC_S_SERVER_TOO_BUSY)
	{
		throw NetworkError(ECategory::Network, ESeverity::Critical, raserror, 
			QString("RasApi: Dial failed because RPC server is busy or unavailable (%1)").arg(raserror));
	}
	else if (raserror != ERROR_SUCCESS)
	{
		throw NetworkError(ECategory::Network, ESeverity::Major, raserror, 
			QString("RasApi: failed to dial '%1': %2 (%3)")
			.arg(mName).arg(QString::fromStdWString(RasApi::EErrorCode::ToString(raserror))).arg(raserror));
	}
}

//--------------------------------------------------------------------------------
void DialupConnection::doDisconnect() throw(...)
{
	DWORD raserror = RasApi::HangUp(mName.toStdWString());

	if (raserror == RPC_S_SERVER_UNAVAILABLE || raserror == RPC_S_SERVER_TOO_BUSY)
	{
		throw NetworkError(ECategory::Network, ESeverity::Critical, raserror, 
			QString("RasApi: HangUp failed because RPC server is busy or unavailable (%1)").arg(raserror));
	}
	else if (raserror != ERROR_SUCCESS)
	{
		throw NetworkError(ECategory::Network, ESeverity::Major, raserror, 
			QString("RasApi: failed to dial '%1': %2 (%3)")
			.arg(mName).arg(QString::fromStdWString(RasApi::EErrorCode::ToString(raserror))).arg(raserror));
	}
}

//--------------------------------------------------------------------------------
bool DialupConnection::doIsConnected() throw(...)
{
	RasApi::EConnectionStatus::Enum status = RasApi::EConnectionStatus::Disconnected;
	
	DWORD raserror = RasApi::GetConnectionStatus(mName.toStdWString(), status);
	
	if (raserror == RPC_S_SERVER_UNAVAILABLE || raserror == RPC_S_SERVER_TOO_BUSY)
	{
		throw NetworkError(ECategory::Network, ESeverity::Critical, raserror, 
			QString("RasApi: GetConnectionStatus failed because RPC server is busy or unavailable (%1)").arg(raserror));
	}
	else if (raserror != ERROR_SUCCESS)
	{
		toLog(LogLevel::Error, QString("GetConnectionStatus for '%1' failed.").arg(mName));
		return false;
	}

	toLog(LogLevel::Debug, QString("RAS connection status: %1").arg(status));

	return status == RasApi::EConnectionStatus::Connected;
}

//--------------------------------------------------------------------------------
bool DialupConnection::doCheckConnection(const CheckUrl & aHost)
{
	return httpCheckMethod(aHost);
}

//----------------------------------------------------------------------------
