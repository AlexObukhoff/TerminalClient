/* @file Виджет автоинкасации */

// Qt
#include "Common/QtHeadersBegin.h"
#include <QtCore/QDateTime>
#include <QtGui/QInputDialog>
#include "Common/QtHeadersEnd.h"

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>

// Project
#include "GUI/ServiceTags.h"
#include "GUI/InputBox.h"
#include "GUI/EncashmentWindow.h"
#include "Backend/MessageBox.h"
#include "Backend/PaymentManager.h"
#include "Backend/HardwareManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "AutoEncashmentWindow.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
namespace CAutoEncashmentWindow
{
	// Максимальное время бездействия в окне автоинкасации.
	const int AutoEncashmentIdleTimeout = 2 * 60 * 1000; // 2 минуты.

	// Интервал обновления текущей даты/времени на экране.
	const int DateTimeRefreshInterval = 60 * 1000; // 1 минута
}

//---------------------------------------------------------------------------
AutoEncashmentWindow::AutoEncashmentWindow(ServiceMenuBackend * aBackend, QWidget * aParent) :
	EncashmentWindow(aBackend, aParent)
{
	ui.setupUi(this);

	ui.lbCurrentDate->setText(QDateTime::currentDateTime().toString("dd/MM/yyyy, hh:mm"));

	connect(ui.btnTestPrinter, SIGNAL(clicked()), SLOT(onTestPrinter()));
	connect(ui.btnEncashment, SIGNAL(clicked()), SLOT(onEncashment()));
	connect(ui.btnEncashmentAndZReport, SIGNAL(clicked()), SLOT(onEncashmentAndZReport()));
	connect(ui.btnEnterServiceMenu, SIGNAL(clicked()), SLOT(onEnterServiceMenu()));
	connect(ui.btnShowHistory, SIGNAL(clicked()), SLOT(onShowHistory()));
	connect(ui.btnExit, SIGNAL(clicked()), SLOT(onExit()));

	connect(ui.stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(onPanelChanged(int)));
	connect(ui.btnBack, SIGNAL(clicked()), SLOT(onBack()));
	
	mIdleTimer.setInterval(CAutoEncashmentWindow::AutoEncashmentIdleTimeout);
	connect(&mIdleTimer, SIGNAL(timeout()), SLOT(onIdleTimeout()));

	mDateTimeTimer.setInterval(CAutoEncashmentWindow::DateTimeRefreshInterval);
	connect(&mDateTimeTimer, SIGNAL(timeout()), SLOT(onDateTimeRefresh()));
	
	PaymentManager *paymentManager = mBackend->getPaymentManager();
	connect(paymentManager, SIGNAL(receiptPrinted(qint64, bool)), SLOT(onPeceiptPrinted(qint64, bool)));

	mHistoryWindow = new EncashmentHistoryWindow(aBackend, this);
	ui.verticalLayoutHistory->insertWidget(0, mHistoryWindow, 1);
}

//---------------------------------------------------------------------------
AutoEncashmentWindow::~AutoEncashmentWindow()
{
}

//---------------------------------------------------------------------------
bool AutoEncashmentWindow::initialize()
{
	mBackend->getTerminalInfo(mTerminalInfo);

	ui.lbTerminalNumber->setText(tr("#terminal_number") + mTerminalInfo[CServiceTags::TerminalNumber].toString());
	ui.lbVersion->setText(tr("#software_version") + mTerminalInfo[CServiceTags::SoftwareVersion].toString());
	ui.stackedWidget->setCurrentIndex(0);

	mIdleTimer.start();
	mDateTimeTimer.start();

	activate();

	return true;
}

//---------------------------------------------------------------------------
bool AutoEncashmentWindow::shutdown()
{
	deactivate();

	return true;
}

//---------------------------------------------------------------------------
bool AutoEncashmentWindow::activate()
{
	connect(mBackend->getHardwareManager(), SIGNAL(deviceStatusChanged(const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)),
		this, SLOT(onDeviceStatusChanged(const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)));

	updateUI();

	return EncashmentWindow::activate();
}

//---------------------------------------------------------------------------
bool AutoEncashmentWindow::deactivate()
{
	disconnect(mBackend->getHardwareManager(), SIGNAL(deviceStatusChanged(const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)),
		this, SLOT(onDeviceStatusChanged(const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)));

	return EncashmentWindow::deactivate();
}

//------------------------------------------------------------------------
void AutoEncashmentWindow::onDeviceStatusChanged(const QString & aConfigName, const QString & aStatusString, const QString & aStatusColor, SDK::Driver::EWarningLevel::Enum aLevel)
{
	Q_UNUSED(aConfigName);
	Q_UNUSED(aStatusString);
	Q_UNUSED(aStatusColor);
	Q_UNUSED(aLevel);

	updateUI();
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::updateUI()
{
	ui.btnEncashment->setEnabled(true);

	ui.btnEncashmentAndZReport->setEnabled(mBackend->getHardwareManager()->isFiscalPrinterPresent(true));

	mBackend->getPaymentManager()->useHardwareFiscalPrinter(mBackend->getHardwareManager()->isFiscalPrinterPresent(false));
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onEncashment()
{
	mEncashmentWithZReport = false;

	doEncashment();
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onTestPrinter()
{
	auto paymentManager = mBackend->getPaymentManager();
	bool isPrinterOK = paymentManager->canPrint(PPSDK::CReceiptType::Encashment);

	if (!isPrinterOK)
	{
		MessageBox::warning(tr("#printer_failed"));

		return;
	}

	paymentManager->printTestPage();
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onEncashmentAndZReport()
{
	mEncashmentWithZReport = true;

	doEncashment();
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onEnterServiceMenu()
{
	mIdleTimer.stop();

	QVariantMap params;
	params["name"] = "service_menu";
	mBackend->sendEvent(SDK::PaymentProcessor::EEventType::StartScenario, params);

	deactivate();
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onShowHistory()
{
	ui.stackedWidget->setCurrentIndex(1);
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onBack()
{
	ui.stackedWidget->setCurrentIndex(0);
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onExit()
{
	mIdleTimer.stop();

	QVariantMap params;
	params["signal"] = "close";
	mBackend->sendEvent(SDK::PaymentProcessor::EEventType::UpdateScenario, params);

	deactivate();
}

//------------------------------------------------------------------------
void AutoEncashmentWindow::onDateTimeRefresh()
{
	ui.lbCurrentDate->setText(QDateTime::currentDateTime().toString("dd/MM/yyyy, hh:mm"));
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onIdleTimeout()
{
	if (mInputBox)
	{
		mInputBox->deleteLater();
		mInputBox = nullptr;
	}

	MessageBox::hide();

	mBackend->toLog("Timeout AutoEncashment in service_menu scenario.");

	onExit();
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onPanelChanged(int aIndex)
{
	if (aIndex != 1)
	{
		return;
	}

	mHistoryWindow->updateHistory();
}

//---------------------------------------------------------------------------