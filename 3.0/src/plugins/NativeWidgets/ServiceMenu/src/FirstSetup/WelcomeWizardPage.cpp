/* @file Окно приветствия. */

// Проект
#include "WizardContext.h"
#include "WelcomeWizardPage.h"

//----------------------------------------------------------------------------
WelcomeWizardPage::WelcomeWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent)
	: WizardPageBase(aBackend, aParent)
{
	setupUi(this);

	connect(btnSetup, SIGNAL(clicked()), SLOT(onRunSetup()));
}

//---------------------------------------------------------------------------
bool WelcomeWizardPage::initialize()
{
	return true;
}

//---------------------------------------------------------------------------
bool WelcomeWizardPage::shutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool WelcomeWizardPage::activate()
{
	return true;
}

//---------------------------------------------------------------------------
bool WelcomeWizardPage::deactivate()
{
	return true;
}

//---------------------------------------------------------------------------
void WelcomeWizardPage::onRunSetup()
{
	emit pageEvent(CWizardContext::RunSetup, true);
}

//---------------------------------------------------------------------------
