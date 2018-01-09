/* @file Окно визарда. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QFrame>
#include <Common/QtHeadersEnd.h>

// Project
#include "GUI/IServiceWindow.h"

//------------------------------------------------------------------------
class WizardPageBase : public QFrame, public ServiceWindowBase
{
	Q_OBJECT

public:
	WizardPageBase(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

signals:
	void pageEvent(const QString & aContext, bool aFlag);
};

//------------------------------------------------------------------------
