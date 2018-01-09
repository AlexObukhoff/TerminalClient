/* @file Реализация общих функций сетевого соединения. */

// Qt headers
#include "Common/QtHeadersBegin.h"
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include <QtNetwork/QNetworkInterface>
#include "Common/QtHeadersEnd.h"

// Common headers
#include <Connection/NetworkError.h>
#include <Connection/IConnection.h>

// Project headers
#include "RasWrapper.h"

#include "DialupConnection.h"
#include "LocalConnection.h"

//--------------------------------------------------------------------------------
namespace CConnection
{
	const int DialTryCount = 1;

	const QString AllUsersProfileVariable = "ALLUSERSPROFILE=";
	const QString Win2000PhonebookPath = "\\Application Data\\Microsoft\\Network\\Connections\\Pbk\\Rasphone.pbk";
}

ILog * IConnection::mLog = nullptr;

//--------------------------------------------------------------------------------
/// Создание экземпляра соединения.
IConnection * IConnection::create(const QString & aName, EConnectionTypes::Enum aType, NetworkTaskManager * aNetwork, ILog * aLog)
{
	IConnection::mLog = aLog;
	
	switch (aType)
	{
		case EConnectionTypes::Dialup:
		{
			return new DialupConnection(aName, aNetwork, aLog);
		}
	}

	return new LocalConnection(aName, aNetwork, aLog);
}

//--------------------------------------------------------------------------------
/// Поиск всех установленных в системе модемов.
QStringList IConnection::getModems()
{
	QStringList modems;
	RasApi::Device device;
	RasApi::DeviceEnumerator denum;

	if (denum.isValid())
	{
		while (denum.getDevice(device))
		{
			if (RasApi::EDeviceType::ToEnum(device.type()) == RasApi::EDeviceType::Modem)
			{
				modems.append(QString::fromStdWString(device.name()));
			}
		}
	}
	else
	{
		throw NetworkError(ECategory::Network, ESeverity::Major, denum.getLastError(), 
			QString("RasApi: device enumerating failed (%1)").arg(denum.getLastError()));
	}

	return modems;
}

//--------------------------------------------------------------------------------
/// Поиск всех установленных в системе сетевых интерфейсов.
QStringList IConnection::getInterfaces()
{
	QStringList interfaces;

	foreach (QNetworkInterface intf, QNetworkInterface::allInterfaces())
	{
		interfaces.append(intf.humanReadableName());
	}

	return interfaces;
}

//--------------------------------------------------------------------------------
/// Список всех удалённых соединений в системе.
QStringList IConnection::getRemoteConnections()
{
	QStringList connections;
	RasApi::PhonebookEntryName entryName;
	RasApi::PhonebookEntryEnumerator eenum;

	if (eenum.isValid())
	{
		while (eenum.getEntry(entryName))
		{
			connections.append(QString::fromStdWString(entryName.name()));
		}
	}
	else
	{
		throw NetworkError(ECategory::Network, ESeverity::Major, eenum.getLastError(), 
			QString("RasApi: connections enumerating failed (%1)").arg(eenum.getLastError()));
	}

	return connections;
}

//--------------------------------------------------------------------------------
/// Список всех локальных соединений в системе.
QStringList IConnection::getLocalConnections()
{
	// UNDONE
	mLog->write(LogLevel::Warning, "Retreiving the list of local connections is not implemented.");

	return QStringList();
}

//--------------------------------------------------------------------------------
/// Создать dialup соединение
void IConnection::createDialupConnection(const QString & aName, const QString & aPhone,
	const QString & aLogin, const QString & aPassword, const QString & aDevice)
{
	RasApi::PhonebookEntryName entryName;
	entryName.setIsSystem(false);
	entryName.setName(aName.toStdWString());

	// Для Win2000 процесс запущенный от LocalSystem ищет телефонную книгу в 
	// несуществующей папке, поэтому перенаправляем его на AllUsers
	if (QSysInfo::WindowsVersion == QSysInfo::WV_2000)
	{
		QString path;

		foreach (QString var, QProcess::systemEnvironment())
		{
			if (var.startsWith(CConnection::AllUsersProfileVariable))
			{
				path = var.mid(CConnection::AllUsersProfileVariable.length()) + CConnection::Win2000PhonebookPath;
				break;
			}
		}

		entryName.setPhonebookPath(path.toStdWString());

		IConnection::mLog->write(LogLevel::Error,
			QString("Win2000 phonebook path workaround applied: %1").arg(path));
	}

	// Проверяем нет ли уже такой записи
	DWORD raserror = RasApi::ValidatePhonebookEntryName(entryName);

	if (raserror == RPC_S_SERVER_UNAVAILABLE || raserror == RPC_S_SERVER_TOO_BUSY)
	{
		throw NetworkError(ECategory::Network, ESeverity::Critical, raserror, 
			QString("RasApi: ValidatePhonebookEntryName failed because RPC server is busy or unavailable (%1)").arg(raserror));
	}
	else if (raserror != ERROR_SUCCESS)
	{
		throw NetworkError(ECategory::Network, ESeverity::Major, raserror, 
			QString("RasApi: ValidatePhonebookEntryName failed: %1 (%2)").
				arg(QString::fromStdWString(RasApi::EErrorCode::ToString(raserror))).arg(raserror));
	}

	// Заполняем параметры
	RasApi::PhonebookEntry entry;

	if (entry.getLastError() == RPC_S_SERVER_UNAVAILABLE || entry.getLastError() == RPC_S_SERVER_TOO_BUSY)
	{
		throw NetworkError(ECategory::Network, ESeverity::Critical, entry.getLastError(), 
			QString("RasApi: failed to declare PhonebookEntry because RPC server is busy or unavailable (%1)").arg(entry.getLastError()));
	}
	else if (entry.getLastError() != ERROR_SUCCESS)
	{
		throw NetworkError(ECategory::Network, ESeverity::Major, entry.getLastError(),
			QString("RasApi: failed to declare PhonebookEntry: %1 (%2)").
				arg(QString::fromStdWString(RasApi::EErrorCode::ToString(entry.getLastError()))).arg(entry.getLastError()));
	}

	entry.setLocalPhoneNumber(aPhone.toStdWString());
	entry.setDeviceName(aDevice.toStdWString());
	entry.setDeviceType(RasApi::EDeviceType::Modem);
	entry.setNetProtocols(RasApi::ENetworkProtocol::Ip);
	entry.setFramingProtocol(RasApi::EFramingProtocol::Ppp);
	entry.setIdleDisconnectSeconds(static_cast<size_t>(RasApi::EIdleDisconnect::Disabled));
	entry.setRedialCount(CConnection::DialTryCount);
	entry.setPhonebookEntryType(RasApi::EPhonebookEntry::Phone);
	entry.setEncriptionType(RasApi::EEncryptionType::Optional);

	entry.setOptions(
		RasApi::EConnectionOption::RemoteDefaultGateway |
		RasApi::EConnectionOption::DisableLcpExtensions |
		RasApi::EConnectionOption::ModemLights |
		RasApi::EConnectionOption::SecureLocalFiles);

	entry.setOptions2(
		RasApi::EConnectionOption2::Internet | 
		RasApi::EConnectionOption2::SecureFileAndPrint | 
		RasApi::EConnectionOption2::SecureClientForMSNet |
		RasApi::EConnectionOption2::DisableNbtOverIP |
		RasApi::EConnectionOption2::DontNegotiateMultilink);

	// Создаём
	raserror = RasApi::CreateNewPhonebookEntry(entryName, entry);

	if (raserror == RPC_S_SERVER_UNAVAILABLE || raserror == RPC_S_SERVER_TOO_BUSY)
	{
		throw NetworkError(ECategory::Network, ESeverity::Critical, raserror, 
			QString("RasApi: CreateNewPhonebookEntry failed because RPC server is busy or unavailable (%1)").arg(raserror));
	}
	else if (raserror != ERROR_SUCCESS)
	{
		throw NetworkError(ECategory::Network, ESeverity::Major, raserror,
			QString("RasApi: CreateNewPhonebookEntry failed: %1 (%2)").
				arg(QString::fromStdWString(RasApi::EErrorCode::ToString(raserror))).arg(raserror));
	}

	// Установка параметров соединения
	RasApi::DialParams dialParams;

	dialParams.setPhoneNumber(aPhone.toStdWString());
	dialParams.setUserName(aLogin.toStdWString());
	dialParams.setPassword(aPassword.toStdWString());

	raserror = RasApi::SetEntryDialParams(entryName, dialParams);

	if (raserror == RPC_S_SERVER_UNAVAILABLE || raserror == RPC_S_SERVER_TOO_BUSY)
	{
		throw NetworkError(ECategory::Network, ESeverity::Critical, raserror, 
			QString("RasApi: SetEntryDialParams failed because RPC server is busy or unavailable (%1)").arg(raserror));
	}
	else if (raserror != ERROR_SUCCESS)
	{
		throw NetworkError(ECategory::Network, ESeverity::Major, raserror,
			QString("RasApi: SetEntryDialParams failed: %1 (%2)").
				arg(QString::fromStdWString(RasApi::EErrorCode::ToString(raserror))).arg(raserror));
	}
}

//--------------------------------------------------------------------------------
void IConnection::removeDialupConnection(const QString & aName)
{
	RasApi::PhonebookEntryName entryName;
	entryName.setIsSystem(false);
	entryName.setName(aName.toStdWString());

	// Для Win2000 процесс запущенный от LocalSystem ищет телефонную книгу в 
	// несуществующей папке, поэтому перенаправляем его на AllUsers
	if (QSysInfo::WindowsVersion == QSysInfo::WV_2000)
	{
		QString path;

		foreach (QString var, QProcess::systemEnvironment())
		{
			if (var.startsWith(CConnection::AllUsersProfileVariable))
			{
				path = var.mid(CConnection::AllUsersProfileVariable.length()) + CConnection::Win2000PhonebookPath;
				break;
			}
		}

		entryName.setPhonebookPath(path.toStdWString());

		IConnection::mLog->write(LogLevel::Error,
			QString("Win2000 phonebook path workaround applied: %1").arg(path));
	}

	// Удаляем
	DWORD raserror = RasApi::RemovePhonebookEntry(entryName);

	if (raserror == RPC_S_SERVER_UNAVAILABLE || raserror == RPC_S_SERVER_TOO_BUSY)
	{
		throw NetworkError(ECategory::Network, ESeverity::Critical, raserror, 
			QString("RasApi: RemovePhonebookEntry failed because RPC server is busy or unavailable (%1)").arg(raserror));
	}
	else if (raserror != ERROR_SUCCESS)
	{
		throw NetworkError(ECategory::Network, ESeverity::Major, raserror,
			QString("RasApi: RemovePhonebookEntry failed: %1 (%2)").
				arg(QString::fromStdWString(RasApi::EErrorCode::ToString(raserror))).arg(raserror));
	}
}

//--------------------------------------------------------------------------------
QString IConnection::getModemInfo(const QString & aName) throw(...)
{
	return QString("Port: %1").arg(QString::fromStdWString(RasApi::getAttachedTo(aName.toStdWString())));
}

//--------------------------------------------------------------------------------
