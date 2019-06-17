/* @file Окно логов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtGui/QKeyEvent>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Project
#include "Backend/ServiceMenuBackend.h"
#include "LogsServiceWindow.h"

//------------------------------------------------------------------------
LogsServiceWindow::LogsServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent)
	: QFrame(aParent),
	  ServiceWindowBase(aBackend)
{
	setupUi(this);
	
	connect(btnShowLog, SIGNAL(clicked()), this, SLOT(onShowLogButtonClicked()));
	connect(lvLogsList, SIGNAL(itemSelectionChanged()), this, SLOT(logsSelectionChanged()));

	connect(btnScrollHome, SIGNAL(clicked()), lvLog, SLOT(scrollToTop()));
	connect(btnScrollUp, SIGNAL(clicked()), this, SLOT(onScrollUpClicked()));
	connect(btnScrollDown, SIGNAL(clicked()), this, SLOT(onScrollDownClicked()));
	connect(btnScrollEnd, SIGNAL(clicked()), lvLog, SLOT(scrollToBottom()));

	connect(btnScrollHomeLogList, SIGNAL(clicked()), lvLogsList, SLOT(scrollToTop()));
	connect(btnScrollUpLogList, SIGNAL(clicked()), this, SLOT(onScrollUpLogListClicked()));
	connect(btnScrollDownLogList, SIGNAL(clicked()), this, SLOT(onScrollDownLogListClicked()));
	connect(btnScrollEndLogList, SIGNAL(clicked()), lvLogsList, SLOT(scrollToBottom()));

	connect(btnCloseLog, SIGNAL(clicked()), this, SLOT(onCloseLogClicked()));

	lvLog->setModel(&mModel);

	QString logPath(QString("%1/../logs").arg(static_cast<SDK::PaymentProcessor::TerminalSettings *>(mBackend->getCore()->getSettingsService()->
		getAdapter(SDK::PaymentProcessor::CAdapterNames::TerminalAdapter))->getAppEnvironment().userDataPath));

	QDir logDir(logPath, "*.log", QDir::Name, QDir::Files);
	foreach (QFileInfo fileInfo, logDir.entryInfoList())
	{
		mLogs.insert(fileInfo.fileName(), fileInfo.filePath());
	}

	stackedWidget->setCurrentIndex(0);
}

//------------------------------------------------------------------------
bool LogsServiceWindow::activate()
{
	lvLogsList->clear();
	lvLogsList->insertItems(0, mLogs.keys());
	return true;
}

//------------------------------------------------------------------------
bool LogsServiceWindow::deactivate()
{
	return true;
}

//------------------------------------------------------------------------
bool LogsServiceWindow::initialize()
{
	return true;
}

//------------------------------------------------------------------------
bool LogsServiceWindow::shutdown()
{
	return true;
}

//------------------------------------------------------------------------
void LogsServiceWindow::onShowLogButtonClicked()
{
	if (lvLogsList->selectedItems().size() > 0)
	{
		QFile logFile(mLogs.value(lvLogsList->selectedItems().first()->text()));

		if (logFile.open(QIODevice::ReadOnly))
		{
			QTextStream stream(&logFile);
			stream.setCodec("utf-8");

			QStringList splitted = stream.readAll().split("\r\n", QString::SkipEmptyParts);

			mModel.setStringList(splitted);
			lvLog->scrollToTop();

			stackedWidget->setCurrentIndex(1);

			labelLogName->setText(lvLogsList->selectedItems().first()->text());
		}
	}
}

//------------------------------------------------------------------------
void LogsServiceWindow::onCloseLogClicked()
{
	stackedWidget->setCurrentIndex(0);
}

//------------------------------------------------------------------------
void LogsServiceWindow::onScrollDownClicked()
{
	scrollPgDown(lvLog);
}

//------------------------------------------------------------------------
void LogsServiceWindow::onScrollUpClicked()
{
	scrollPgUp(lvLog);
}

//------------------------------------------------------------------------
void LogsServiceWindow::logsSelectionChanged()
{
	btnShowLog->setEnabled(lvLogsList->selectedItems().size() > 0);
}

//------------------------------------------------------------------------
void LogsServiceWindow::onScrollUpLogListClicked()
{
	scrollPgUp(lvLogsList);
}

//------------------------------------------------------------------------
void LogsServiceWindow::onScrollDownLogListClicked()
{
	scrollPgDown(lvLogsList);
}

//------------------------------------------------------------------------
