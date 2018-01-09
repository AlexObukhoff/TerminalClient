/* @file Окно визарда. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <Common/QtHeadersEnd.h>

// Проект
#include "WizardPage.h"

//------------------------------------------------------------------------
WizardPageBase::WizardPageBase(ServiceMenuBackend * aBackend, QWidget * aParent)
	: ServiceWindowBase(aBackend),
		QFrame(aParent)
{
}

//------------------------------------------------------------------------
