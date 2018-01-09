/* @file Окно настройки ключей. */

#pragma once

// Проект
#include "WizardPage.h"

class KeysWindow;
class IServiceBackend;

//------------------------------------------------------------------------
class KeysWizardPage : public WizardPageBase
{
	Q_OBJECT

public:
	KeysWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();

	virtual bool activate();
	virtual bool deactivate();

private slots:
	void onBeginGenerating();
	void onEndGenerating();

	void onError(QString aError);

private:
	KeysWindow * mKeysWindow;
};

//------------------------------------------------------------------------
