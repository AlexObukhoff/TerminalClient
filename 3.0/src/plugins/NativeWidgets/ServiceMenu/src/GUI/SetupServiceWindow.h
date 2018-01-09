/* @file Окно настроек. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include "ui_SetupServiceWindow.h"
#include <Common/QtHeadersEnd.h>

// Проект
#include "IServiceWindow.h"

//------------------------------------------------------------------------
class SetupServiceWindow : public QFrame, public ServiceWindowBase, public Ui::SetupServiceWindow
{
	Q_OBJECT

public:
	SetupServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();
	virtual bool activate();
	virtual bool deactivate();

protected slots:
	// Активация/деактивация вкладок
	void onCurrentPageChanged(int aIndex);

private:
	int mCurrentPageIndex;
};

//------------------------------------------------------------------------
