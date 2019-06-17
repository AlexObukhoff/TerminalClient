/* @file Окошко для генерации ключей в сервисном меню и первоначальной настройке. */

// boost
#include <boost/bind.hpp>
#include <boost/ref.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtConcurrentRun>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Project
#include "Backend/KeysManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "Backend/MessageBox.h"
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
	connect(cbKeypairChange, SIGNAL(stateChanged(int)), this, SLOT(onCheckedKeyPairChanged(int)));
	connect(&mGenerateTaskWatcher, SIGNAL(finished()), SLOT(onGenerateTaskFinished()));

	cbKeypairChange->setEnabled(mBackend->getKeysManager()->allowAnyKeyPair());
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

	cbKeypairChange->setChecked(Qt::Unchecked);
	frameKeyPair->setEnabled(false);
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

	QString keyPair = sbKeypairNumber->text();
	bool rewriteExistNumber = true;

	if (cbKeypairChange->checkState() == Qt::Checked && !keyPair.isEmpty())
	{
		int num = keyPair.toInt();

		if (mBackend->getKeysManager()->getLoadedKeys().contains(num))
		{
			rewriteExistNumber = MessageBox::question(tr("#keypair_already_exist"));
		}
	}
	else
	{
		keyPair = "0";
	}

	if (!rewriteExistNumber)
	{
		return;
	}

	mTaskParameters.clear();

	mTaskParameters[CServiceTags::Login] = login->text();
	mTaskParameters[CServiceTags::Password] = password->text();	
	mTaskParameters[CServiceTags::KeyPairNumber] = keyPair;
	mTaskParameters[CServiceTags::KeyPairDescription] = description->text();

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
void KeysWindow::onCheckedKeyPairChanged(int aState)
{
	frameKeyPair->setEnabled(aState > 0);
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
