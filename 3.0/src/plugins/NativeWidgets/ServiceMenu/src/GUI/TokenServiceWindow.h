/* @file Окно настроек. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include "ui_TokenServiceWindow.h"
#include <Common/QtHeadersEnd.h>

// Проект
#include "TokenWindow.h"
#include "IServiceWindow.h"

//------------------------------------------------------------------------
class TokenServiceWindow : public QFrame, public ServiceWindowBase, protected Ui::TokenServiceWindow
{
	Q_OBJECT

public:
	TokenServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

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
	TokenWindow * mWindow;
};

//------------------------------------------------------------------------
