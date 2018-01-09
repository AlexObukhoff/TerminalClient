/* @file Окно инкассации. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include "ui_EncashmentServiceWindow.h"
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/WarningLevel.h>

// Project
#include "GUI/EncashmentWindow.h"

class ServiceMenuBackend;

//------------------------------------------------------------------------
class EncashmentServiceWindow : public EncashmentWindow
{
	Q_OBJECT

public:
	EncashmentServiceWindow (ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();
	virtual bool activate();
	virtual bool deactivate();

protected:
	virtual void updateUI();
	void updateInfo();

private slots:
	void doPayload();
	void onPrintBalance();
	void onDeviceStatusChanged(const QString & aConfigName, const QString & aStatusString, const QString & aStatusColor, SDK::Driver::EWarningLevel::Enum aLevel);

private:
	Ui::EncashmentServiceWindow ui;

private:
	QVariantMap mPayloadSettings;
	QString mPayloadSettingsPath;
	ServiceMenuBackend * mBackend;
};

//------------------------------------------------------------------------
