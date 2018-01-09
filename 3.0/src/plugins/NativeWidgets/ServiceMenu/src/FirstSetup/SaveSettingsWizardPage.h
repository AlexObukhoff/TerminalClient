/* @file Окно сохранения настроек. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include "ui_SaveSettingsWizardPage.h"
#include <Common/QtHeadersEnd.h>

// Проект
#include "WizardPage.h"

//---------------------------------------------------------------------------
class SaveSettingsWizardPage : public WizardPageBase, protected Ui::SaveSettingsWizardPage
{
	Q_OBJECT

public:
	SaveSettingsWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();

	virtual bool activate();
	virtual bool deactivate();

private slots:
	void onSave();
	void onFinish();

private:
	void showError(const QString & aContext, const QString & aError);
};

//---------------------------------------------------------------------------
