/* @file Окошко для отображения истории инкассаций. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSignalMapper>
#include "ui_encashmentHistoryWindow.h"
#include <Common/QtHeadersEnd.h>

// Project
class ServiceMenuBackend;

//------------------------------------------------------------------------
class EncashmentHistoryWindow : public QWidget, protected Ui_EncashmentHistoryWindow
{
	Q_OBJECT

public:
	EncashmentHistoryWindow(ServiceMenuBackend * aBackend, QWidget * aParent);

	virtual ~EncashmentHistoryWindow();

public slots:
	void updateHistory();

private slots:
	void printEncashment(int aIndex);

protected:
	ServiceMenuBackend * mBackend;
	QList<QWidget *> mWidgets;
	QSignalMapper * mSignalMapper;
};

//------------------------------------------------------------------------
