/* @file Окно с локальным соединением. */

// Проект
#include "Backend/ServiceMenuBackend.h"
#include "UnmanagedConnectionWindow.h"
#include "SIPStyle.h"

//---------------------------------------------------------------------------
UnmanagedConnectionWindow::UnmanagedConnectionWindow(QWidget * aParent)
	: QFrame(aParent)
{
	setupUi(this);

	foreach (QLineEdit * le, findChildren<QLineEdit *>())
	{
		le->setStyle(new SIPStyle);
	}

	leProxyPort->setValidator(new QIntValidator(0, 65534, this));

	connect(cbProxyType, SIGNAL(currentIndexChanged(int)), SLOT(onProxyTypeChanged(int)));
	connect(btnTestConnection, SIGNAL(clicked()), SLOT(onTestConnection()));

	connect(leProxyAddress, SIGNAL(textChanged(const QString &)), SLOT(onTextChanged(const QString &)));
	connect(leProxyUser, SIGNAL(textChanged(const QString &)), SLOT(onTextChanged(const QString &)));
	connect(leProxyPassword, SIGNAL(textChanged(const QString &)), SLOT(onTextChanged(const QString &)));
	connect(leProxyPort, SIGNAL(textChanged(const QString &)), SLOT(onTextChanged(const QString &)));

	cbProxyType->addItem(tr("#type_noproxy"), QNetworkProxy::NoProxy);
	cbProxyType->addItem(tr("#type_http"), QNetworkProxy::HttpProxy);
	cbProxyType->addItem(tr("#type_socks5"), QNetworkProxy::Socks5Proxy);
}

//---------------------------------------------------------------------------
void UnmanagedConnectionWindow::initialize(const QNetworkProxy & aProxy)
{
	leProxyAddress->setText(aProxy.hostName());
	leProxyPort->setText(QString::number(aProxy.port()));
	leProxyUser->setText(aProxy.user());
	leProxyPassword->setText(aProxy.password());
	cbProxyType->setCurrentIndex(cbProxyType->findData(aProxy.type()) == -1 ? 0 : cbProxyType->findData(aProxy.type()));
}

//---------------------------------------------------------------------------
void UnmanagedConnectionWindow::toggleProxy(bool aEnabled)
{
	leProxyAddress->setEnabled(aEnabled);
	leProxyPort->setEnabled(aEnabled);
	leProxyUser->setEnabled(aEnabled);
	leProxyPassword->setEnabled(aEnabled);

	lbProxyAddress->setEnabled(aEnabled);
	lbProxyPort->setEnabled(aEnabled);
	lbProxyUser->setEnabled(aEnabled);
	lbProxyPassword->setEnabled(aEnabled);
}

//---------------------------------------------------------------------------
QNetworkProxy UnmanagedConnectionWindow::getUserSelection() const
{
	QNetworkProxy proxy;

	proxy.setType(static_cast<QNetworkProxy::ProxyType>((cbProxyType->itemData(cbProxyType->currentIndex()).toInt())));
	proxy.setHostName(leProxyAddress->text());
	proxy.setPort(static_cast<quint16>(leProxyPort->text().toUInt()));
	proxy.setUser(leProxyUser->text());
	proxy.setPassword(leProxyPassword->text());

	return proxy;
}

//---------------------------------------------------------------------------
void UnmanagedConnectionWindow::onTextChanged(const QString & /*aText*/)
{
	emit userSelectionChanged();
}

//---------------------------------------------------------------------------
void UnmanagedConnectionWindow::onTestConnection()
{
	emit testConnection(getUserSelection());
}

//---------------------------------------------------------------------------
void UnmanagedConnectionWindow::onProxyTypeChanged(int aIndex)
{
	toggleProxy(cbProxyType->itemData(aIndex).toInt() != QNetworkProxy::NoProxy);
	emit userSelectionChanged();
}

//---------------------------------------------------------------------------
