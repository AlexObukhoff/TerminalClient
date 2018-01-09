/* @file Окно выбора типа сети. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include "ui_NetworkWizardPage.h"
#include <Common/QtHeadersEnd.h>

// Проект
#include "WizardPage.h"

//---------------------------------------------------------------------------
class NetworkWizardPage : public WizardPageBase, protected Ui::NetworkWizardPage
{
	Q_OBJECT

public:
	NetworkWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();

	virtual bool activate();
	virtual bool deactivate();

private slots:
	void onChooseDialup();
	void onChooseUnmanaged();
};

//---------------------------------------------------------------------------
