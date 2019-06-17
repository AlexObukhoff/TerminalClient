/* @file Окно визарда. */

// Проект
#include "WizardPage.h"

//------------------------------------------------------------------------
WizardPageBase::WizardPageBase(ServiceMenuBackend * aBackend, QWidget * aParent)
	: ServiceWindowBase(aBackend),
		QFrame(aParent)
{
}

//------------------------------------------------------------------------
