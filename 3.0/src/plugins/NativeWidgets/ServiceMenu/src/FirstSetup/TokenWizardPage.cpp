/* @file Окно настройки RuToken. */

// Project
#include "MessageBox/MessageBox.h"
#include "Backend/ServiceMenuBackend.h"
#include "Backend/KeysManager.h"
#include "GUI/TokenWindow.h"
#include "TokenWizardPage.h"

//------------------------------------------------------------------------
TokenWizardPage::TokenWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent) : 
	WizardPageBase(aBackend, aParent),
	mUIUpdateTimer(0)
{
	mTokenWindow = new TokenWindow(aBackend, this);

	connect(mTokenWindow, SIGNAL(beginFormat()), SLOT(onBeginFormat()));
	connect(mTokenWindow, SIGNAL(endFormat()), SLOT(onEndFormat()));
	connect(mTokenWindow, SIGNAL(error(QString)), SLOT(onError(QString)));

	setLayout(new QHBoxLayout(this));
	layout()->setSpacing(0);
	layout()->setMargin(0);
	layout()->addWidget(mTokenWindow);
}

//------------------------------------------------------------------------
bool TokenWizardPage::initialize()
{
	auto status = mBackend->getKeysManager()->tokenStatus();

	mTokenWindow->initialize(status);
	emit pageEvent("#can_proceed", status.isOK());

	if (!status.isOK())
	{
		mUIUpdateTimer = startTimer(1000);
	}

	return true;
}

//------------------------------------------------------------------------
bool TokenWizardPage::shutdown()
{
	killTimer(mUIUpdateTimer);
	return true;
}

//------------------------------------------------------------------------
bool TokenWizardPage::activate()
{
	mTokenWindow->initialize(mBackend->getKeysManager()->tokenStatus());

	return true;
}

//------------------------------------------------------------------------
bool TokenWizardPage::deactivate()
{
	killTimer(mUIUpdateTimer);
	return true;
}

//----------------------------------------------------------------------------
void TokenWizardPage::onBeginFormat()
{
	GUI::MessageBox::hide();
	GUI::MessageBox::wait(tr("#format_token"));

	killTimer(mUIUpdateTimer);

	mTokenWindow->doFormat();
}

//----------------------------------------------------------------------------
void TokenWizardPage::onEndFormat()
{
	GUI::MessageBox::hide();

	mUIUpdateTimer = startTimer(1000);
}

//----------------------------------------------------------------------------
void TokenWizardPage::onError(QString aError)
{
	GUI::MessageBox::hide();
	GUI::MessageBox::critical(aError);

	mUIUpdateTimer = startTimer(1000);
}

//------------------------------------------------------------------------
void TokenWizardPage::timerEvent(QTimerEvent *)
{
	auto status = mBackend->getKeysManager()->tokenStatus();

	mTokenWindow->initialize(status);

	emit pageEvent("#can_proceed", status.isOK());
}

//------------------------------------------------------------------------
