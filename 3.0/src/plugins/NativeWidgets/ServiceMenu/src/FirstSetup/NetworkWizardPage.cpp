/* @file Окно выбора типа сети. */

// Проект
#include "WizardContext.h"
#include "NetworkWizardPage.h"

//----------------------------------------------------------------------------
NetworkWizardPage::NetworkWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent)
	: WizardPageBase(aBackend, aParent)
{
	setupUi(this);

	connect(btnUnmanaged, SIGNAL(clicked()), SLOT(onChooseUnmanaged()));
	connect(btnDialup, SIGNAL(clicked()), SLOT(onChooseDialup()));
}

//------------------------------------------------------------------------
bool NetworkWizardPage::initialize()
{
	return true;
}

//------------------------------------------------------------------------
bool NetworkWizardPage::shutdown()
{
	return true;
}

//------------------------------------------------------------------------
bool NetworkWizardPage::activate()
{
	return true;
}

//------------------------------------------------------------------------
bool NetworkWizardPage::deactivate()
{
	return true;
}

//----------------------------------------------------------------------------
void NetworkWizardPage::onChooseDialup()
{
	emit pageEvent(CWizardContext::SetupDialup, true);
}

//----------------------------------------------------------------------------
void NetworkWizardPage::onChooseUnmanaged()
{
	emit pageEvent(CWizardContext::SetupUnmanaged, true);
}

//----------------------------------------------------------------------------
