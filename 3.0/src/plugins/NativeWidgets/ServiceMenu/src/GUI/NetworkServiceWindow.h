/* @file Окно сетевых настроек. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFutureWatcher>
#include <QtNetwork/QNetworkProxy>
#include <QtGui/QAbstractButton>
#include <QtGui/QButtonGroup>
#include "ui_NetworkServiceWindow.h"
#include <Common/QtHeadersEnd.h>

// Проект
#include "DialupConnectionWindow.h"
#include "UnmanagedConnectionWindow.h"
#include "IServiceWindow.h"

//------------------------------------------------------------------------
class NetworkServiceWindow : public QWidget, public ServiceWindowBase, public Ui::NetworkServiceWindow
{
	Q_OBJECT

public:
	NetworkServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();
	virtual bool activate();
	virtual bool deactivate();

private slots:
	void onChangeConnectionType(QAbstractButton * aButton);

	void onCreateDialupConnection(const QString & aConnection, const QString & aNetworkDevice);
	void onTestDialupConnection(const QString & aConnection);
	void onRemoveDialupConnection(const QString & aConnection);
	void onTestUnmanagedConnection(QNetworkProxy aProxy);
	void onTestFinished();

private:
	QButtonGroup mTypeButtonGroup;
	DialupConnectionWindow * mDialupWindow;
	UnmanagedConnectionWindow * mUnmanagedWindow;

	QFutureWatcher<bool> mTaskWatcher;
	QString mConnectionError;
	ServiceMenuBackend * mBackend;
};

//------------------------------------------------------------------------
