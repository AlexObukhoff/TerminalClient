/* @file Окно настроек. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include "ui_KeysServiceWindow.h"
#include <Common/QtHeadersEnd.h>

// Проект
#include "KeysWindow.h"
#include "IServiceWindow.h"

//------------------------------------------------------------------------
class KeysServiceWindow : public QFrame, public ServiceWindowBase, protected Ui::KeysServiceWindow
{
	Q_OBJECT

public:
	KeysServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();
	virtual bool activate();
	virtual bool deactivate();

private slots:
	void onBeginGenerating();
	void onEndGenerating();

	void onError(QString aError);

private:
	KeysWindow * mWindow;
};

//------------------------------------------------------------------------
