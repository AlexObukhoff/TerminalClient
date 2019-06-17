/* @file Окно с модемным соединением. */

// boost
#include <boost/bind.hpp>
#include <boost/ref.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtConcurrentRun>
#include <Common/QtHeadersEnd.h>

// Модули
#include <SDK/PaymentProcessor/Connection/ConnectionTypes.h>

// Проект
#include "Backend/MessageBox.h"
#include "Backend/NetworkManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "GUI/ServiceTags.h"
#include "GUI/DialupConnectionWindow.h"
#include "DialupWizardPage.h"

//------------------------------------------------------------------------
DialupWizardPage::DialupWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent)
	: WizardPageBase(aBackend, aParent)
{
	mConnectionWindow = new DialupConnectionWindow(this);
	mConnectionWindow->setParent(this);
	setLayout(new QHBoxLayout(this));
	layout()->setSpacing(0);
	layout()->setMargin(0);
	layout()->addWidget(mConnectionWindow);
	
	connect(mConnectionWindow, SIGNAL(createConnection(const QString &, const QString &)),
		SLOT(onCreateConnection(const QString &, const QString &))); 
	connect(mConnectionWindow, SIGNAL(testConnection(const QString &)), SLOT(onTestConnection(const QString &)));
	connect(mConnectionWindow, SIGNAL(removeConnection(const QString &)), SLOT(onRemoveConnection(const QString &)));
	connect(mConnectionWindow, SIGNAL(userSelectionChanged(const QString &)), SLOT(onSelectionChanged(const QString &)));

	connect(&mTaskWatcher, SIGNAL(finished()), SLOT(onTestFinished()));
}

//------------------------------------------------------------------------
bool DialupWizardPage::initialize()
{
	mConnectionWindow->initialize();
	mConnectionWindow->fillTemplateList(mBackend->getNetworkManager()->getConnectionTemplates());
	mConnectionWindow->fillModemList(mBackend->getNetworkManager()->getModems());

	QVariantMap networkInfo;
	mBackend->getNetworkManager()->getNetworkInfo(networkInfo);
	mConnectionWindow->fillConnectionList(mBackend->getNetworkManager()->getRemoteConnections(),
		networkInfo[CServiceTags::Connection].toString());

	emit pageEvent("#can_proceed", false);

	return true;
}

//------------------------------------------------------------------------
bool DialupWizardPage::shutdown()
{
	mTaskWatcher.waitForFinished();

	return true;
}

//------------------------------------------------------------------------
bool DialupWizardPage::activate()
{
	return true;
}

//------------------------------------------------------------------------
bool DialupWizardPage::deactivate()
{
	SDK::PaymentProcessor::SConnection connection;

	connection.type = EConnectionTypes::Dialup;
	connection.name = mConnectionWindow->getUserSelection();

	mBackend->getNetworkManager()->setConnection(connection);

	return true;
}

//------------------------------------------------------------------------
void DialupWizardPage::onSelectionChanged(const QString & /*aSelectedConnection*/)
{
	emit pageEvent("#can_proceed", false);
}

//------------------------------------------------------------------------
void DialupWizardPage::onCreateConnection(const QString & aConnection, const QString & aNetworkDevice)
{
	SDK::PaymentProcessor::SConnection connection;

	connection.type = EConnectionTypes::Dialup;
	connection.name = aConnection;

	if (mBackend->getNetworkManager()->createDialupConnection(connection, aNetworkDevice))
	{
		mConnectionWindow->fillConnectionList(mBackend->getNetworkManager()->getRemoteConnections(), aConnection);
		mConnectionWindow->switchToListPage();
	}
	else
	{
		MessageBox::warning(tr("#failed_create_connection"));
	}
}

//------------------------------------------------------------------------
void DialupWizardPage::onTestConnection(const QString & aConnection)
{
	SDK::PaymentProcessor::SConnection connection;

	connection.type = EConnectionTypes::Dialup;
	connection.name = aConnection;
	mBackend->getNetworkManager()->setConnection(connection);

	MessageBox::wait(tr("#testing_connection"));

	mTaskWatcher.setFuture(QtConcurrent::run(boost::bind(&NetworkManager::testConnection, mBackend->getNetworkManager(), boost::ref(mConnectionError))));
}

//------------------------------------------------------------------------
void DialupWizardPage::onRemoveConnection(const QString & aConnection)
{
	SDK::PaymentProcessor::SConnection connection;

	connection.type = EConnectionTypes::Dialup;
	connection.name = aConnection;

	if (mBackend->getNetworkManager()->removeDialupConnection(connection))
	{
		mConnectionWindow->fillConnectionList(mBackend->getNetworkManager()->getRemoteConnections(), aConnection);
		mConnectionWindow->switchToListPage();
	}
	else
	{
		MessageBox::warning(tr("#failed_remove_connection"));
	}
}

//---------------------------------------------------------------------------
void DialupWizardPage::onTestFinished()
{
	MessageBox::hide();
	MessageBox::info(mTaskWatcher.result() ? tr("#connection_test_ok") : tr("#connection_test_failed") + "\n" + mConnectionError);

	emit pageEvent("#can_proceed", mTaskWatcher.result());
}

//------------------------------------------------------------------------
