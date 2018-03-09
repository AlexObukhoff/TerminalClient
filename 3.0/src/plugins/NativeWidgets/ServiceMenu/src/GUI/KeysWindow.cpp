/* @file Окошко для генерации ключей в сервисном меню и первоначальной настройке. */

// boost
#include <boost/bind.hpp>
#include <boost/ref.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QUrl>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFuture>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Project
#include "Backend/KeysManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "SIPStyle.h"
#include "ServiceTags.h"
#include "KeysWindow.h"

//------------------------------------------------------------------------
KeysWindow::KeysWindow(ServiceMenuBackend * aBackend, QWidget * aParent)
	: QFrame(aParent),
	  mBackend(aBackend)
{
	setupUi(this);

	foreach (QLineEdit * le, findChildren<QLineEdit *>())
	{
		le->setStyle(new SIPStyle);
	}

	connect(btnCreate, SIGNAL(clicked()), SLOT(onCreateButtonClicked()));
	connect(btnRepeat, SIGNAL(clicked()), SLOT(onRepeatButtonClicked()));

	connect(&mGenerateTaskWatcher, SIGNAL(finished()), SLOT(onGenerateTaskFinished()));
}

//------------------------------------------------------------------------
KeysWindow::~KeysWindow()
{
	if (mGenerateTaskWatcher.isRunning())
	{
		mGenerateTaskWatcher.waitForFinished();
	}	
}

//------------------------------------------------------------------------
void KeysWindow::initialize(bool aHasRuToken, bool aRutokenOK)
{
	swPages->setCurrentWidget(wGeneratePage);

	foreach(QLineEdit * le, findChildren<QLineEdit *>())
	{
		le->setEnabled(!aHasRuToken || (aHasRuToken && aRutokenOK));
	}

	foreach(QPushButton * pb, findChildren<QPushButton *>())
	{
		pb->setEnabled(!aHasRuToken || (aHasRuToken && aRutokenOK));
	}
}

//------------------------------------------------------------------------
bool KeysWindow::save()
{
	if (mGenerateTaskWatcher.isRunning())
	{
		return false;
	}	

	return mBackend->getKeysManager()->saveKey();
}

//------------------------------------------------------------------------
void KeysWindow::doGenerate()
{
	mGenerateTaskWatcher.setFuture(QtConcurrent::run(boost::bind(&KeysManager::generateKey, mBackend->getKeysManager(), boost::ref(mTaskParameters))));
}

//------------------------------------------------------------------------
void KeysWindow::onCreateButtonClicked()
{
	SetStyleSheet(login, login->text().isEmpty() ? CKeysWindow::WarningStyleSheet : CKeysWindow::DefaultStyleSheet);
	SetStyleSheet(password, password->text().isEmpty() ? CKeysWindow::WarningStyleSheet : CKeysWindow::DefaultStyleSheet);

	if (login->text().isEmpty() || password->text().isEmpty())
	{
		return;
	}

	mTaskParameters.clear();

	mTaskParameters[CServiceTags::Login] = login->text();
	mTaskParameters[CServiceTags::Password] = password->text();

	emit beginGenerating();
}

//------------------------------------------------------------------------
void KeysWindow::onRepeatButtonClicked()
{
	login->clear();
	password->clear();

	swPages->setCurrentWidget(wGeneratePage);
}

//------------------------------------------------------------------------
void KeysWindow::onGenerateTaskFinished()
{
	if (mGenerateTaskWatcher.result())
	{
		lbAp->setText(mBackend->getKeysManager()->getAP());
		lbSd->setText(mBackend->getKeysManager()->getSD());
		lbOp->setText(mBackend->getKeysManager()->getOP());

		swPages->setCurrentWidget(wResultsPage);

		emit endGenerating();
	}
	else
	{
		emit error(mTaskParameters[CServiceTags::Error].toString());
	}
}

//------------------------------------------------------------------------
