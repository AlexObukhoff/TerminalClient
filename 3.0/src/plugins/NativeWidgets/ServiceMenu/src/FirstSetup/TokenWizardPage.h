/* @file Окно настройки RuToken. */

#pragma once

// Проект
#include "WizardPage.h"

class TokenWindow;
class IServiceBackend;

//------------------------------------------------------------------------
class TokenWizardPage : public WizardPageBase
{
	Q_OBJECT

public:
	TokenWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();

	virtual bool activate();
	virtual bool deactivate();

private slots:
	void onBeginFormat();
	void onEndFormat();

	void onError(QString aError);

protected:
	int mUIUpdateTimer;
	void timerEvent(QTimerEvent *);

private:
	TokenWindow * mTokenWindow;
};

//------------------------------------------------------------------------
