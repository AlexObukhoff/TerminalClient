/* @file Окно с локальным соединением. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFuture>
#include <QtWidgets/QHBoxLayout>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Connection/Connection.h>
#include <SDK/PaymentProcessor/Connection/ConnectionTypes.h>

// Проект
#include "Backend/MessageBox.h"
#include "Backend/NetworkManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "GUI/ServiceTags.h"
#include "GUI/UnmanagedConnectionWindow.h"
#include "UnmanagedWizardPage.h"

//---------------------------------------------------------------------------
UnmanagedWizardPage::UnmanagedWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent)
	:	WizardPageBase(aBackend, aParent),
		mConnectionWindow(new UnmanagedConnectionWindow)
{
	mConnectionWindow->setParent(this);
	setLayout(new QHBoxLayout(this));
	layout()->setSpacing(0);
	layout()->setMargin(0);
	layout()->addWidget(mConnectionWindow);

	connect(mConnectionWindow, SIGNAL(testConnection(QNetworkProxy)), SLOT(onTestConnection(QNetworkProxy)));
	connect(mConnectionWindow, SIGNAL(userSelectionChanged()), SLOT(onUserSelectionChanged()));
	connect(&mTaskWatcher, SIGNAL(finished()), SLOT(onTestFinished()));
}

//------------------------------------------------------------------------
bool UnmanagedWizardPage::initialize()
{
	mConnectionSettings[CServiceTags::ConnectionType] = EConnectionTypes::Unmanaged;
	mConnectionSettings[CServiceTags::Connection]     = "unmanaged connection";

	QVariantMap result;
	mBackend->getNetworkManager()->getNetworkInfo(result);

	// Заполняем хосты для пинга и временные интервалы
	foreach (const QString & param, result.keys())
	{
		if (param.contains(CServiceTags::CheckHost) || (param == CServiceTags::CheckInterval))
		{
			mStaticParameters.insert(param, result[param]);
		}
	}
	
	QNetworkProxy proxy(QNetworkProxy::NoProxy);

	if (result.contains(CServiceTags::ProxyType))
	{
		proxy.setType(QNetworkProxy::ProxyType(result[CServiceTags::ProxyType].toInt()));
		proxy.setHostName(result[CServiceTags::ProxyAddress].toString());
		proxy.setPort(static_cast<quint16>(result[CServiceTags::ProxyPort].toUInt()));
		proxy.setUser(result[CServiceTags::ProxyUser].toString());
		proxy.setPassword(result[CServiceTags::ProxyPassword].toString());
	}

	mConnectionWindow->initialize(proxy);

	emit pageEvent("#can_proceed", false);

	return true;
}

//------------------------------------------------------------------------
bool UnmanagedWizardPage::shutdown()
{
	mTaskWatcher.waitForFinished();

	return true;
}

//------------------------------------------------------------------------
bool UnmanagedWizardPage::activate()
{
	return true;
}

//------------------------------------------------------------------------
bool UnmanagedWizardPage::deactivate()
{
	SDK::PaymentProcessor::SConnection connection;

	connection.name = "unmanaged connection";
	connection.type = EConnectionTypes::Unmanaged;

	QNetworkProxy proxy(QNetworkProxy::NoProxy);

	if (mConnectionSettings.contains(CServiceTags::ProxyType))
	{
		proxy.setType(QNetworkProxy::ProxyType(mConnectionSettings[CServiceTags::ProxyType].toInt()));
		proxy.setHostName(mConnectionSettings[CServiceTags::ProxyAddress].toString());
		proxy.setPort(static_cast<quint16>(mConnectionSettings[CServiceTags::ProxyPort].toUInt()));
		proxy.setUser(mConnectionSettings[CServiceTags::ProxyUser].toString());
		proxy.setPassword(mConnectionSettings[CServiceTags::ProxyPassword].toString());
	}

	connection.proxy = proxy;

	mBackend->getNetworkManager()->setConnection(connection);

	return true;
}

//---------------------------------------------------------------------------
void UnmanagedWizardPage::onUserSelectionChanged()
{
	emit pageEvent("#can_proceed", false);
}

//---------------------------------------------------------------------------
void UnmanagedWizardPage::onTestConnection(QNetworkProxy aProxy)
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

	mBackend->getNetworkManager()->setConnection(connection);

	MessageBox::wait(tr("#testing_connection"));

	mTaskWatcher.setFuture(QtConcurrent::run(mBackend->getNetworkManager(), &NetworkManager::testConnection, QString()));
}

//---------------------------------------------------------------------------
void UnmanagedWizardPage::onTestFinished()
{
	MessageBox::hide();
	MessageBox::info(mTaskWatcher.result() ? tr("#connection_test_ok") : tr("#connection_test_failed"));

	// сохраняем в mConnectionSettings хорошие настройки прокси
	if (mTaskWatcher.result())
	{
		auto proxy = mBackend->getNetworkManager()->getConnection().proxy;

		mConnectionSettings[CServiceTags::ProxyType] = proxy.type();
		mConnectionSettings[CServiceTags::ProxyAddress] = proxy.hostName();
		mConnectionSettings[CServiceTags::ProxyPort] = proxy.port();
		mConnectionSettings[CServiceTags::ProxyUser] = proxy.user();
		mConnectionSettings[CServiceTags::ProxyPassword] = proxy.password();
	}

	emit pageEvent("#can_proceed", mTaskWatcher.result());
}

//------------------------------------------------------------------------
