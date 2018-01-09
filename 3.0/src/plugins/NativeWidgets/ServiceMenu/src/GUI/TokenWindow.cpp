/* @file Окошко для генерации ключей в сервисном меню и первоначальной настройке. */

// boost
#include <boost/bind.hpp>
#include <boost/ref.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QFuture>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Project
#include "Backend/KeysManager.h"
#include "Backend/ServiceMenuBackend.h"

#include "ServiceTags.h"
#include "TokenWindow.h"

//------------------------------------------------------------------------
TokenWindow::TokenWindow(ServiceMenuBackend * aBackend, QWidget * aParent)
	: QFrame(aParent),
	  mBackend(aBackend)
{
	setupUi(this);

	connect(btnFormat, SIGNAL(clicked()), SLOT(onFormatButtonClicked()));

	connect(&mFormatTaskWatcher, SIGNAL(finished()), SLOT(onFormatTaskFinished()));
}

//------------------------------------------------------------------------
TokenWindow::~TokenWindow()
{
	if (mFormatTaskWatcher.isRunning())
	{
		mFormatTaskWatcher.waitForFinished();
	}	
}

//------------------------------------------------------------------------
void TokenWindow::initialize(const CCrypt::TokenStatus & aStatus)
{
	updateUI(aStatus);
}

//------------------------------------------------------------------------
void TokenWindow::doFormat()
{
	mFormatTaskWatcher.setFuture(QtConcurrent::run(mBackend->getKeysManager(), &KeysManager::formatToken));
}

//------------------------------------------------------------------------
void TokenWindow::onFormatButtonClicked()
{
	mTaskParameters.clear();

	emit beginFormat();
}

//------------------------------------------------------------------------
void TokenWindow::onFormatTaskFinished()
{
	if (mFormatTaskWatcher.result())
	{
		updateUI(mBackend->getKeysManager()->tokenStatus());

		emit endFormat();
	}
	else
	{
		emit error(tr("#error_format_token"));
	}
}

//------------------------------------------------------------------------
void TokenWindow::updateUI(const CCrypt::TokenStatus & aStatus)
{
	labelName->setText(aStatus.name.isEmpty() ? tr("#empty") : aStatus.name);

	QString status;

	if (aStatus.available && aStatus.initialized)
	{
		status = tr("#ok");
		btnFormat->setEnabled(false);
	}
	else if (aStatus.available)
	{
		status = tr("#not_initialised");
		btnFormat->setEnabled(true);
	}
	else
	{
		status = tr("#none");
		btnFormat->setEnabled(false);
	}

	labelStatus->setText(status);
}

//------------------------------------------------------------------------
