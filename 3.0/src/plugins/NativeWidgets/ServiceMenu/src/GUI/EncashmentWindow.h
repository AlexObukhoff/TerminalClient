/* @file Базовый виджет для инкасации */
#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QWidget>
#include <QtCore/QTimer>
#include <Common/QtHeadersEnd.h>

// Project
#include "InputBox.h"
#include "EncashmentHistoryWindow.h"
#include "IServiceWindow.h"

//---------------------------------------------------------------------------
class ServiceMenuBackend;

//---------------------------------------------------------------------------
class EncashmentWindow : public QWidget, public ServiceWindowBase
{
	Q_OBJECT

public:
	EncashmentWindow(ServiceMenuBackend * aBackend, QWidget * aParent = 0);
	virtual ~EncashmentWindow();

public:
	virtual bool activate() { return true; }
	virtual bool deactivate();

protected slots:
	void doEncashment();
	virtual bool doEncashmentProcess();
	void onPrintZReport();
	void onPeceiptPrinted(qint64 aPaymentId, bool aErrorHappened);

protected:
	virtual void updateUI() = 0;

protected:
	QString mMessageSuccess;
	QString mMessageError;
	bool mEncashmentWithZReport;

protected:
	QTimer mIdleTimer;
	InputBox * mInputBox;
	EncashmentHistoryWindow * mHistoryWindow;
};

//---------------------------------------------------------------------------
