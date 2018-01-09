/* @file Окно с модемным соединением. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFutureWatcher>
#include <Common/QtHeadersEnd.h>

// Проект
#include "WizardPage.h"

class DialupConnectionWindow;

//------------------------------------------------------------------------
namespace CDialupWizardPage
{
	/// Минимальное врея, когда будет показан экран ожидания при тесте соединения
	const int MinimumPingTime = 1000;
}

//------------------------------------------------------------------------
class DialupWizardPage : public WizardPageBase
{
	Q_OBJECT

public:
	DialupWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();

	virtual bool activate();
	virtual bool deactivate();

private slots:
	void onSelectionChanged(const QString & aSelectedConnection);
	void onCreateConnection(const QString & aConnection, const QString & aNetworkDevice);
	void onTestConnection(const QString & aConnection);
	void onRemoveConnection(const QString & aConnection);
	void onTestFinished();

private:
	QFutureWatcher<bool> mTaskWatcher;
	QString mConnectionError;
	DialupConnectionWindow * mConnectionWindow;
};

//------------------------------------------------------------------------
