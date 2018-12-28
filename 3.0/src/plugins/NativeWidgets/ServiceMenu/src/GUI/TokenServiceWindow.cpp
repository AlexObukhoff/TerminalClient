/* @file Окно настроек. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <Common/QtHeadersEnd.h>

// Модули
#include <Common/Application.h>
#include <SDK/PaymentProcessor/Core/ICore.h>

// Проект
#include "MessageBox/MessageBox.h"
#include "Backend/KeysManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "TokenServiceWindow.h"

//------------------------------------------------------------------------
TokenServiceWindow::TokenServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent)
	: QFrame(aParent),
	  ServiceWindowBase(aBackend)
{
	setupUi(this);

	mWindow = new TokenWindow(aBackend, this);

	connect(mWindow, SIGNAL(beginFormat()), SLOT(onBeginFormat()));
	connect(mWindow, SIGNAL(endFormat()), SLOT(onEndFormat()));
	connect(mWindow, SIGNAL(error(QString)), SLOT(onError(QString)));

	mWindow->setParent(this);
	wContainer->setLayout(new QHBoxLayout);
	wContainer->layout()->setSpacing(0);
	wContainer->layout()->setMargin(0);
	wContainer->layout()->addWidget(mWindow);
}

//------------------------------------------------------------------------
bool TokenServiceWindow::activate()
{
	mWindow->initialize(mBackend->getKeysManager()->tokenStatus());

	mUIUpdateTimer = startTimer(1000);

	return true;
}

//------------------------------------------------------------------------
bool TokenServiceWindow::deactivate()
{
	killTimer(mUIUpdateTimer);

	return true;
}

//------------------------------------------------------------------------
bool TokenServiceWindow::initialize()
{
	mWindow->initialize(mBackend->getKeysManager()->tokenStatus());

	return true;
}

//------------------------------------------------------------------------
bool TokenServiceWindow::shutdown()
{
	killTimer(mUIUpdateTimer);
	return true;
}

//------------------------------------------------------------------------
void TokenServiceWindow::onBeginFormat()
{
	if (GUI::MessageBox::question(tr("#question_format_token_warning")))
	{
		GUI::MessageBox::hide();
		GUI::MessageBox::wait(tr("#format_token"));
		
		killTimer(mUIUpdateTimer);

		mWindow->doFormat();
	}
}

//------------------------------------------------------------------------
void TokenServiceWindow::onEndFormat()
{
	GUI::MessageBox::hide();

	mUIUpdateTimer = startTimer(1000);
}

//------------------------------------------------------------------------
void TokenServiceWindow::onError(QString aError)
{
	GUI::MessageBox::hide();
	GUI::MessageBox::critical(aError);

	mUIUpdateTimer = startTimer(1000);
}

//------------------------------------------------------------------------
void TokenServiceWindow::timerEvent(QTimerEvent *)
{
	mWindow->initialize(mBackend->getKeysManager()->tokenStatus());
}

//------------------------------------------------------------------------
