/* @file Декорация визарда для локального соединения. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVariantMap>
#include <QtCore/QFutureWatcher>
#include <QtNetwork/QNetworkProxy>
#include <Common/QtHeadersEnd.h>

// Проект
#include "WizardPage.h"

class UnmanagedConnectionWindow;

//---------------------------------------------------------------------------
class UnmanagedWizardPage :	public WizardPageBase
{
	Q_OBJECT

public:
	UnmanagedWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();

	virtual bool activate();
	virtual bool deactivate();

private slots:
	void onUserSelectionChanged();
	void onTestConnection(QNetworkProxy aProxy);
	void onTestFinished();

private:
	QVariantMap mConnectionSettings;
	UnmanagedConnectionWindow * mConnectionWindow;
	QFutureWatcher<bool> mTaskWatcher;
	QVariantMap   mStaticParameters;
};

//---------------------------------------------------------------------------
