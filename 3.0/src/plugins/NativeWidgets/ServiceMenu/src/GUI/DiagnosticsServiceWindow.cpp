/* @file Окно диагностики. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>

// Project
#include "Backend/PaymentManager.h"
#include "Backend/HardwareManager.h"
#include "Backend/NetworkManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "ServiceTags.h"
#include "DeviceStatusWindow.h"
#include "DiagnosticsServiceWindow.h"

//------------------------------------------------------------------------
DiagnosticsServiceWindow::DiagnosticsServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent)
	: QFrame(aParent),
	  ServiceWindowBase(aBackend),
	  mSpacerItem(0)
{
	setupUi(this);

	connect(mBackend->getHardwareManager(), SIGNAL(deviceStatusChanged(const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)), 
		this, SLOT(onDeviceStatusChanged(const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)));

	connect(btnInfoPanel, SIGNAL(clicked()), this, SLOT(onClickedEncashmentInfo()));
	connect(btnTestServer, SIGNAL(clicked()), this, SLOT(onClickedTestServer()));
	connect(&mTaskWatcher, SIGNAL(finished()), SLOT(onTestServerFinished()));
	connect(btnResetReject, SIGNAL(clicked()), this, SLOT(onClickedResetReject()));
	connect(btnResetReceipts, SIGNAL(clicked()), this, SLOT(onClickedResetReceipts()));

	// TODO Реализовать функционал
	lbTitleZReportCount->hide();
	lbZReportCount->hide();
	lbTitleSessionStatus->hide();
	lbSessionStatus->hide();

	onClickedTestServer();
}

//------------------------------------------------------------------------
bool DiagnosticsServiceWindow::activate()
{
	onClickedEncashmentInfo();
	
	foreach (DeviceStatusWindow * widget, mDeviceStatusWidget.values())
	{
		widget->deleteLater();
	}

	mDeviceStatusWidget.clear();

	vlTestWidgets->removeItem(mSpacerItem);
	delete mSpacerItem;
	mSpacerItem = 0;

	// Получаем список устройств из конфигов.
	QStringList configNames = mBackend->getHardwareManager()->getConfigurations();

	foreach (const QString config, configNames)
	{
		DeviceStatusWindow * dtw = new DeviceStatusWindow(mBackend, config, this);
		vlTestWidgets->addWidget(dtw->getWidget());

		mDeviceStatusWidget[config] = dtw;
	}

	mBackend->getHardwareManager()->updateStatuses();

	mSpacerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
	vlTestWidgets->layout()->addItem(mSpacerItem);

	updateInfoPanel();

	return true;
}

//------------------------------------------------------------------------
bool DiagnosticsServiceWindow::deactivate()
{
	return true;
}

//------------------------------------------------------------------------
bool DiagnosticsServiceWindow::initialize()
{
	return true;
}

//------------------------------------------------------------------------
bool DiagnosticsServiceWindow::shutdown()
{
	mTaskWatcher.waitForFinished();

	return true;
}

//------------------------------------------------------------------------
void DiagnosticsServiceWindow::onDeviceStatusChanged(const QString & aConfigurationName, const QString & aNewStatus, const QString & aStatusColor, SDK::Driver::EWarningLevel::Enum aLevel)
{
	DeviceStatusWindow * widget = mDeviceStatusWidget[aConfigurationName];
	if (widget)
	{
		widget->updateDeviceStatus(aNewStatus, aStatusColor, aLevel);
	}
}

//------------------------------------------------------------------------
void DiagnosticsServiceWindow::onClickedEncashmentInfo()
{
	bool state = btnInfoPanel->isChecked() == true;

	state ? btnInfoPanel->setText(tr("#title_turn_off")) : btnInfoPanel->setText(tr("#title_turn_on"));
	scrollArea_2->setVisible(state);
}

//------------------------------------------------------------------------
void DiagnosticsServiceWindow::updateInfoPanel()
{
	QVariantMap result;

	foreach(SDK::PaymentProcessor::IService * service, mBackend->getCore()->getServices())
	{
		result.unite(service->getParameters());
	}

	lbSimBalance->setText(result[SDK::PaymentProcessor::CServiceParameters::Networking::SimBalance].toString());
	lbRejectedBills->setText(QString::number(result[SDK::PaymentProcessor::CServiceParameters::Funds::RejectCount].toInt()));
	lbUnprocessedPayments->setText(QString::number(result[SDK::PaymentProcessor::CServiceParameters::Payment::UnprocessedPaymentCount].toInt()));
	lbPrintedReceipts->setText(QString::number(result[SDK::PaymentProcessor::CServiceParameters::Printing::ReceiptCount].toInt()));
	lbRestartPerDay->setText(QString::number(result[SDK::PaymentProcessor::CServiceParameters::Terminal::RestartCount].toInt()));
	lbPaymentsPerDay->setText(QString::number(result[SDK::PaymentProcessor::CServiceParameters::Payment::PaymentsPerDay].toInt()));

	QVariant param(result[SDK::PaymentProcessor::CServiceParameters::Printing::SessionStatus]);
	lbSessionStatus->setText(param.isNull() ? "-/-" : param.toString());
	lbSessionStatus->setEnabled(!param.isNull());
	lbTitleSessionStatus->setEnabled(!param.isNull());

	param = result[SDK::PaymentProcessor::CServiceParameters::Printing::SessionStatus];
	lbZReportCount->setText(param.isNull() ? "-/-" : QString::number(param.toInt()));
	lbZReportCount->setEnabled(!param.isNull());
	lbTitleZReportCount->setEnabled(!param.isNull());

	QVariantMap cashInfo = mBackend->getPaymentManager()->getBalanceInfo();
	lbLastEncashmentDate->setText(cashInfo[CServiceTags::LastEncashmentDate].toString());

	bool isRoleTechician = mBackend->getUserRole() == CServiceTags::UserRole::RoleTechnician;
	lbAmount->setText(isRoleTechician ? "-/-" : cashInfo[CServiceTags::CashAmount].toString());
	lbNotesCount->setText(isRoleTechician ? "-/-" : QString::number(cashInfo[CServiceTags::NoteCount].toInt()));
	lbCoinsCount->setText(isRoleTechician ? "-/-" : QString::number(cashInfo[CServiceTags::CoinCount].toInt()));
}

//---------------------------------------------------------------------------
void DiagnosticsServiceWindow::resetParameter(const QString & aParameterName)
{
	QSet<QString> parameters;
	parameters << aParameterName;

	foreach(SDK::PaymentProcessor::IService * service, mBackend->getCore()->getServices())
	{
		service->resetParameters(parameters);
	}
}

//---------------------------------------------------------------------------
void DiagnosticsServiceWindow::onClickedTestServer()
{
	btnTestServer->setEnabled(false);
	lbNetworkStatus->setText(tr("#connection_checking_status"));
	mTaskWatcher.setFuture(QtConcurrent::run(mBackend->getNetworkManager(), &NetworkManager::testConnection, QString()));
}

//---------------------------------------------------------------------------
void DiagnosticsServiceWindow::onTestServerFinished()
{
	btnTestServer->setEnabled(true);
	mTaskWatcher.result() ? 
		lbNetworkStatus->setText(tr("#connection_test_ok")) : 
		lbNetworkStatus->setText(tr("#connection_test_failed"));
}

//---------------------------------------------------------------------------
void DiagnosticsServiceWindow::onClickedResetReject()
{
	resetParameter(SDK::PaymentProcessor::CServiceParameters::Funds::RejectCount);
	updateInfoPanel();
}

//---------------------------------------------------------------------------
void DiagnosticsServiceWindow::onClickedResetReceipts()
{
	resetParameter(SDK::PaymentProcessor::CServiceParameters::Printing::ReceiptCount);
	updateInfoPanel();
}

//------------------------------------------------------------------------
