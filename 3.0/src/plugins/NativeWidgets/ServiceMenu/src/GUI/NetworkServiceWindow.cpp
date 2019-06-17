/* @file Окно сетевых настроек. */

// boost
#include <boost/bind.hpp>
#include <boost/ref.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtConcurrentRun>
#include <Common/QtHeadersEnd.h>

// Модули
#include <Common/ILog.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Connection/ConnectionTypes.h>

// Проект
#include "Backend/MessageBox.h"
#include "Backend/NetworkManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "ServiceTags.h"
#include "NetworkServiceWindow.h"

//------------------------------------------------------------------------
NetworkServiceWindow::NetworkServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent)
	: QWidget(aParent),
	  ServiceWindowBase(aBackend),
	  mBackend(aBackend),
	  mDialupWindow(0),
	  mUnmanagedWindow(0)
{
	setupUi(this);

	mTypeButtonGroup.addButton(rbDialupLink);
	mTypeButtonGroup.addButton(rbUnmanagedLink);

	connect(&mTypeButtonGroup,
		SIGNAL(buttonClicked(QAbstractButton *)), SLOT(onChangeConnectionType(QAbstractButton *)));
}

//------------------------------------------------------------------------
bool NetworkServiceWindow::activate()
{
	return true;
}

//------------------------------------------------------------------------
bool NetworkServiceWindow::deactivate()
{
	SDK::PaymentProcessor::SConnection connection;

	if (rbUnmanagedLink->isChecked())
	{
		connection.name = "unmanaged connection";
		connection.type = EConnectionTypes::Unmanaged;
		connection.proxy = mUnmanagedWindow->getUserSelection();
	}
	else if (rbDialupLink->isChecked())
	{
		connection.name = mDialupWindow->getUserSelection();
		connection.type = EConnectionTypes::Dialup;
	}

	mBackend->getNetworkManager()->setConnection(connection);
	mBackend->saveConfiguration();

	return true;
}

//------------------------------------------------------------------------
bool NetworkServiceWindow::initialize()
{
	connect(&mTaskWatcher, SIGNAL(finished()), SLOT(onTestFinished()));

	QVariantMap networkInfo;
	mBackend->getNetworkManager()->getNetworkInfo(networkInfo);

	EConnectionTypes::Enum type = EConnectionTypes::Enum(networkInfo[CServiceTags::ConnectionType].toInt());

	// Инициализация окна с настройками модемного соединения
	mDialupWindow = new DialupConnectionWindow(this);
	mDialupWindow->initialize();
	mDialupWindow->fillTemplateList(mBackend->getNetworkManager()->getConnectionTemplates());
	mDialupWindow->fillModemList(mBackend->getNetworkManager()->getModems());
	mDialupWindow->fillConnectionList(mBackend->getNetworkManager()->getRemoteConnections(),
		networkInfo[CServiceTags::Connection].toString());

	connect(mDialupWindow, SIGNAL(createConnection(const QString &, const QString &)), 
		SLOT(onCreateDialupConnection(const QString &, const QString &)));
	connect(mDialupWindow, SIGNAL(testConnection(const QString &)), 
		SLOT(onTestDialupConnection(const QString &)));
	connect(mDialupWindow, SIGNAL(removeConnection(const QString &)), 
		SLOT(onRemoveDialupConnection(const QString &)));

	swContainer->addWidget(mDialupWindow);

	// Инициализация окна с настройками соединения по локальной сети
	QNetworkProxy proxy(
		static_cast<QNetworkProxy::ProxyType>(networkInfo[CServiceTags::ProxyType].toInt()),
		networkInfo[CServiceTags::ProxyAddress].toString(),
		static_cast<quint16>(networkInfo[CServiceTags::ProxyPort].toUInt()),
		networkInfo[CServiceTags::ProxyUser].toString(),
		networkInfo[CServiceTags::ProxyPassword].toString());

	mUnmanagedWindow = new UnmanagedConnectionWindow(this);
	mUnmanagedWindow->initialize(proxy);

	connect(mUnmanagedWindow, SIGNAL(testConnection(QNetworkProxy)), SLOT(onTestUnmanagedConnection(QNetworkProxy)));

	swContainer->addWidget(mUnmanagedWindow);

	// Контролы переключения типа соединения
	if (type == EConnectionTypes::Unmanaged)
	{
		rbUnmanagedLink->click();
	}
	else if (type == EConnectionTypes::Dialup)
	{
		rbDialupLink->click();
	}

	return true;
}

//------------------------------------------------------------------------
bool NetworkServiceWindow::shutdown()
{
	mTaskWatcher.waitForFinished();

	return true;
}

//------------------------------------------------------------------------
void NetworkServiceWindow::onChangeConnectionType(QAbstractButton * aButton)
{
	if (aButton == rbDialupLink)
	{
		swContainer->setCurrentWidget(mDialupWindow);
	}
	else
	{
		swContainer->setCurrentWidget(mUnmanagedWindow);
	}
}

//------------------------------------------------------------------------
void NetworkServiceWindow::onCreateDialupConnection(const QString & aConnection, const QString & aNetworkDevice)
{
	SDK::PaymentProcessor::SConnection connection;

	connection.type = EConnectionTypes::Dialup;
	connection.name = aConnection;

	mBackend->getNetworkManager()->createDialupConnection(connection, aNetworkDevice);
	
	QVariantMap connections;
	foreach (QString c, mBackend->getNetworkManager()->getRemoteConnections())
	{
		connections.insert(c, 0);
	}

	mDialupWindow->fillConnectionList(connections.keys(), aConnection);
	mDialupWindow->switchToListPage();
}

//------------------------------------------------------------------------
void NetworkServiceWindow::onTestDialupConnection(const QString & aConnection)
{
	SDK::PaymentProcessor::SConnection connection;

	connection.type = EConnectionTypes::Dialup;
	connection.name = aConnection;

	MessageBox::wait(tr("#testing_connection"));

	mBackend->getNetworkManager()->setConnection(connection);
	mTaskWatcher.setFuture(QtConcurrent::run(boost::bind(&NetworkManager::testConnection, mBackend->getNetworkManager(), boost::ref(mConnectionError))));
}

//------------------------------------------------------------------------
void NetworkServiceWindow::onRemoveDialupConnection(const QString & aConnection)
{
	SDK::PaymentProcessor::SConnection connection;

	connection.type = EConnectionTypes::Dialup;
	connection.name = aConnection;

	mBackend->getNetworkManager()->removeDialupConnection(connection);
	
	QVariantMap connections;
	foreach (QString c, mBackend->getNetworkManager()->getRemoteConnections())
	{
		connections.insert(c, 0);
	}

	mDialupWindow->fillConnectionList(connections.keys(), aConnection);
	mDialupWindow->switchToListPage();
}

//---------------------------------------------------------------------------
void NetworkServiceWindow::onTestUnmanagedConnection(QNetworkProxy aProxy)
{
	SDK::PaymentProcessor::SConnection connection;

	connection.name = "unmanaged connection";
	connection.type = EConnectionTypes::Unmanaged;

	QNetworkProxy proxy(QNetworkProxy::NoProxy);

	proxy.setType(aProxy.type());
	proxy.setHostName(aProxy.hostName());
	proxy.setPort(aProxy.port());
	proxy.setUser(aProxy.user());
	proxy.setPassword(aProxy.password());

	connection.proxy = proxy;

	MessageBox::wait(tr("#testing_connection"));

	mBackend->getNetworkManager()->setConnection(connection);
	mTaskWatcher.setFuture(QtConcurrent::run(boost::bind(&NetworkManager::testConnection, mBackend->getNetworkManager(), boost::ref(mConnectionError))));
}

//---------------------------------------------------------------------------
void NetworkServiceWindow::onTestFinished()
{
	MessageBox::hide();
	MessageBox::info(mTaskWatcher.result() ? tr("#connection_test_ok") : tr("#connection_test_failed") + "\n" + mConnectionError);
}

//------------------------------------------------------------------------
