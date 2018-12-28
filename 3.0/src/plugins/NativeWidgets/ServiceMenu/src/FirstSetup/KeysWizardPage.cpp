/* @file Окно настройки ключей. */

// Project
#include "MessageBox/MessageBox.h"
#include "Backend/ServiceMenuBackend.h"
#include "Backend/KeysManager.h"
#include "GUI/KeysWindow.h"
#include "KeysWizardPage.h"

//------------------------------------------------------------------------
KeysWizardPage::KeysWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent)
	: WizardPageBase(aBackend, aParent)
{
	mKeysWindow = new KeysWindow(aBackend, this);

	connect(mKeysWindow, SIGNAL(beginGenerating()), SLOT(onBeginGenerating()));
	connect(mKeysWindow, SIGNAL(endGenerating()), SLOT(onEndGenerating()));
	connect(mKeysWindow, SIGNAL(error(QString)), SLOT(onError(QString)));

	setLayout(new QHBoxLayout(this));
	layout()->setSpacing(0);
	layout()->setMargin(0);
	layout()->addWidget(mKeysWindow);
}

//------------------------------------------------------------------------
bool KeysWizardPage::initialize()
{
	emit pageEvent("#can_proceed", false);

	auto tokenStatus = mBackend->getKeysManager()->tokenStatus();

	mKeysWindow->initialize(tokenStatus.available, tokenStatus.initialized);

	return true;
}

//------------------------------------------------------------------------
bool KeysWizardPage::shutdown()
{
	return true;
}

//------------------------------------------------------------------------
bool KeysWizardPage::activate()
{
	auto tokenStatus = mBackend->getKeysManager()->tokenStatus();

	mKeysWindow->initialize(tokenStatus.available, tokenStatus.initialized);

	return true;
}

//------------------------------------------------------------------------
bool KeysWizardPage::deactivate()
{
	return true;
}

//----------------------------------------------------------------------------
void KeysWizardPage::onBeginGenerating()
{
	GUI::MessageBox::wait(tr("#creating_keys"));
	
	mKeysWindow->doGenerate();
}

//----------------------------------------------------------------------------
void KeysWizardPage::onEndGenerating()
{
	GUI::MessageBox::hide();

	// Сохраняем ключи
	if (!mKeysWindow->save())
	{
		GUI::MessageBox::critical(tr("#cannot_save_keys"));
		emit pageEvent("#can_proceed", false);
	}
	else
	{
		emit pageEvent("#can_proceed", true);
	}
}

//----------------------------------------------------------------------------
void KeysWizardPage::onError(QString aError)
{
	GUI::MessageBox::hide();
	GUI::MessageBox::critical(aError);

	emit pageEvent("#can_proceed", false);
}

//------------------------------------------------------------------------
