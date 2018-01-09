/* @file Окно диагностики. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <QtCore/QFutureWatcher>
#include "ui_DiagnosticsServiceWindow.h"
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/WarningLevel.h>

// Проект
#include "IServiceWindow.h"

class DeviceStatusWindow;

//------------------------------------------------------------------------
class DiagnosticsServiceWindow : public QFrame, public ServiceWindowBase, protected Ui::DiagnosticsServiceWindow
{
	Q_OBJECT

public:
	DiagnosticsServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();
	virtual bool activate();
	virtual bool deactivate();

private:
	void updateInfoPanel();
	void resetParameter(const QString & aParameterName);

private slots:
	void onClickedEncashmentInfo();
	void onDeviceStatusChanged(const QString & aConfigurationName, const QString & aNewStatus, const QString & aStatusColor, SDK::Driver::EWarningLevel::Enum aLevel);
	void onClickedTestServer();
	void onTestServerFinished();
	void onClickedResetReject();
	void onClickedResetReceipts();

private:
	QSpacerItem * mSpacerItem;
	typedef QMap<QString, DeviceStatusWindow *> TDeviceStatusWidget;
	TDeviceStatusWidget mDeviceStatusWidget;
	QFutureWatcher<bool> mTaskWatcher;
};

//------------------------------------------------------------------------
