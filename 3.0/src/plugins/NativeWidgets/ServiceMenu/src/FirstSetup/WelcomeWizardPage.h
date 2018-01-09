/* @file Окно приветствия. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include "ui_WelcomeWizardPage.h"
#include <Common/QtHeadersEnd.h>

// Проект
#include "WizardPage.h"

class ServiceMenuBackend;

//---------------------------------------------------------------------------
class WelcomeWizardPage : public WizardPageBase, protected Ui::WelcomeWizardPage
{
	Q_OBJECT

public:
	WelcomeWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();

	virtual bool activate();
	virtual bool deactivate();

private slots:
	void onRunSetup();
};

//---------------------------------------------------------------------------
