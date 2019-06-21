/* @file Менеджер для работы с сетью */

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Settings/Directory.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <Connection/IConnection.h>

// Project
#include "GUI/ServiceTags.h"

#include "MessageBox/MessageBox.h"
#include "NetworkManager.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
NetworkManager::NetworkManager(SDK::PaymentProcessor::ICore * aCore)
	: mCore(aCore)
{
	mNetworkService = mCore->getNetworkService();
	mTerminalSettings = static_cast<PPSDK::TerminalSettings *>(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));
	mDirectory = static_cast<PPSDK::Directory *>(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::Directory));

	mInitialConnection = mNetworkService->getConnection();
	mSelectedConnection = mInitialConnection;
}

//---------------------------------------------------------------------------
NetworkManager::~NetworkManager()
{
}

//---------------------------------------------------------------------------
bool NetworkManager::isConfigurationChanged() const
{
	return !(mSelectedConnection == mInitialConnection);
}

//------------------------------------------------------------------------
void NetworkManager::resetConfiguration()
{
	mInitialConnection = mSelectedConnection;
}

//---------------------------------------------------------------------------
bool NetworkManager::openConnection(bool aWait)
{
	return mNetworkService->openConnection(aWait);
}

//---------------------------------------------------------------------------
bool NetworkManager::closeConnection()
{
	return mNetworkService->closeConnection();
}

//---------------------------------------------------------------------------
bool NetworkManager::isConnected(bool aUseCache) const
{
	return mNetworkService->isConnected(aUseCache);
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::SConnection NetworkManager::getConnection() const
{
	return mNetworkService->getConnection();
}

//---------------------------------------------------------------------------
void NetworkManager::setConnection(const SDK::PaymentProcessor::SConnection & aConnection)
{
	mSelectedConnection = aConnection;
	mNetworkService->setConnection(aConnection);
	mTerminalSettings->setConnection(aConnection);
}

//---------------------------------------------------------------------------
bool NetworkManager::testConnection(QString & aErrorMessage)
{
	bool result = mNetworkService->testConnection();
	aErrorMessage = mNetworkService->getLastConnectionError().split(":").last();
	return result;
}

//---------------------------------------------------------------------------
QList<QPair<QString, QString> > NetworkManager::getModems() const
{
	QList<QPair<QString, QString> > result;

	try
	{
		foreach (auto modem, IConnection::getModems())
		{
			result << QPair<QString, QString>(modem, IConnection::getModemInfo(modem));
		}
	}
	catch (const NetworkError &)
	{
		GUI::MessageBox::critical(tr("#get_modems_error"));
	}

	return result;
}

//---------------------------------------------------------------------------
QStringList NetworkManager::getInterfaces() const
{
	try
	{
		return IConnection::getInterfaces();
	}
	catch (const NetworkError &)
	{
		GUI::MessageBox::critical(tr("#get_interface_error"));
	}

	return QStringList();
}

//---------------------------------------------------------------------------
QStringList NetworkManager::getRemoteConnections() const
{
	try
	{
		return IConnection::getRemoteConnections();
	}
	catch (const NetworkError &)
	{
		GUI::MessageBox::critical(tr("#get_remote_connections_error"));
	}

	return QStringList();
}

//---------------------------------------------------------------------------
QStringList NetworkManager::getLocalConnections() const
{
	try
	{
		return IConnection::getLocalConnections();
	}
	catch (const NetworkError &)
	{
		GUI::MessageBox::critical(tr("#get_local_connections_error"));
	}

	return QStringList();
}

//---------------------------------------------------------------------------
 QStringList NetworkManager::getConnectionTemplates() const
{
	QStringList nameList;

	foreach (SDK::PaymentProcessor::SConnectionTemplate dialupTemplate, mDirectory->getConnectionTemplates())
	{
		nameList << dialupTemplate.name;
	}

	return nameList;
}

//---------------------------------------------------------------------------
bool NetworkManager::createDialupConnection(const SDK::PaymentProcessor::SConnection & aConnection, const QString & aNetworkDevice)
{
	foreach (PPSDK::SConnectionTemplate connection, mDirectory->getConnectionTemplates())
	{
		if (connection.name == aConnection.name)
		{
			try
			{
				IConnection::createDialupConnection(connection.name, connection.phone, connection.login, connection.password, aNetworkDevice);
				return true;
			}
			catch (const NetworkError & e)
			{
				GUI::MessageBox::critical(e.getSeverity() == ESeverity::Critical ? tr("#phone_entry_call_failed") :  tr("#phone_entry_already_exist\n"));
			}
		}
	}

	return false;
}

//---------------------------------------------------------------------------
bool NetworkManager::removeDialupConnection(const SDK::PaymentProcessor::SConnection & aConnection)
{
	mNetworkService->closeConnection();

	foreach (PPSDK::SConnectionTemplate connection, mDirectory->getConnectionTemplates())
	{
		if (connection.name == aConnection.name)
		{
			try
			{
				IConnection::removeDialupConnection(connection.name);
				return true;
			}
			catch (const NetworkError & e)
			{
				GUI::MessageBox::critical(e.getSeverity() == ESeverity::Critical ? tr("#phone_entry_call_failed") :  tr("#phone_entry_not_exist\n"));
			}
		}
	}

	return false;
}

//---------------------------------------------------------------------------
void NetworkManager::getNetworkInfo(QVariantMap & aResult) const
{
	aResult[CServiceTags::Connection] = mNetworkService->getConnection().name;
	aResult[CServiceTags::ConnectionType] = mNetworkService->getConnection().type;
	aResult[CServiceTags::CheckInterval] = mNetworkService->getConnection().checkInterval;
	aResult[CServiceTags::ConnectionStatus] = mNetworkService->isConnected();

	QNetworkProxy proxy = mNetworkService->getConnection().proxy;

	aResult[CServiceTags::ProxyType] = proxy.type();

	if (proxy.type() != QNetworkProxy::NoProxy)
	{
		aResult[CServiceTags::ProxyAddress] = proxy.hostName();
		aResult[CServiceTags::ProxyPort] = proxy.port();
		aResult[CServiceTags::ProxyUser] = proxy.user();
		aResult[CServiceTags::ProxyPassword] = proxy.password();
	}

	aResult.unite(dynamic_cast<PPSDK::IService *>(mNetworkService)->getParameters());
}

//---------------------------------------------------------------------------

