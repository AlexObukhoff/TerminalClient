/* @file Главное окно сервисного меню. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWidgets/QStatusBar>
#include <Common/QtHeadersEnd.h>

// Модули
#include <SDK/PaymentProcessor/Core/ICore.h>

// Проект
#include "IServiceWindow.h"
#include "ServiceTags.h"
#include "MessageBox/MessageBox.h"
#include "Backend/NetworkManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "MainServiceWindow.h"
#include "DiagnosticsServiceWindow.h"
#include "SetupServiceWindow.h"
#include "EncashmentServiceWindow.h"
#include "PaymentServiceWindow.h"
#include "LogsServiceWindow.h"

namespace CMainServiceWindow
{
	const char * DigitProperty = "cyberDigit";

	// Интервал обновления текущей даты/времени на экране.
	const int DateTimeRefreshInterval = 60 * 1000; // 1 минута

	// Максимальное время бездействия на этапе ввода пароля.
	const int PasswordIdleTimeout = 2 * 60 * 1000; // 2 минуты.

	// Максимальное время бездействия в сервисном меню.
	const int MenuIdleTimeout = 3 * 60 * 1000; /// 3 минуты.

	const QString IdleScenarioName = "idle";

	const int ExitQuestionTimeout = 60; // in sec
}

//------------------------------------------------------------------------
MainServiceWindow::MainServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent)
	: QWidget(aParent),
	mBackend(aBackend),
	mCurrentPageIndex(-1)
{
	setupUi(this);

	swPages->setCurrentWidget(wPasswordPage);

	connect(btnProceedAuth, SIGNAL(clicked()), SLOT(onProceedLogin()));
	connect(btnCancelAuth, SIGNAL(clicked()), SLOT(onCancelAuthorization()));
	connect(btnCloseServiceMenu, SIGNAL(clicked()), SLOT(onCloseServiceMenu()));
	connect(btnRebootTerminal, SIGNAL(clicked()), SLOT(onRebootTerminal()));
	connect(btnToggleLock, SIGNAL(clicked()), SLOT(onToggleLock()));
	connect(btnRebootApplication, SIGNAL(clicked()), this, SLOT(onRebootApplication()));
	connect(btnStopApplication, SIGNAL(clicked()), this, SLOT(onStopApplication()));

	// Кнопки цифровой клавиатуры
	QList<QPushButton *> numericPadButtons = wNumericPad->findChildren<QPushButton *>();
	foreach(QPushButton * button, numericPadButtons)
	{
		connect(button, SIGNAL(clicked()), SLOT(onDigitClicked()));
	}

	connectAllAbstractButtons(this);

	connect(btnBackspace, SIGNAL(clicked()), this, SLOT(onBackspaceClicked()));
	connect(btnClear, SIGNAL(clicked()), this, SLOT(onClearClicked()));

	mIdleTimer.setSingleShot(true);
	mDateTimeTimer.setInterval(CMainServiceWindow::DateTimeRefreshInterval);

	connect(&mIdleTimer, SIGNAL(timeout()), SLOT(onIdleTimeout()));
	connect(&mDateTimeTimer, SIGNAL(timeout()), SLOT(onDateTimeRefresh()));

	connect(lePassword, SIGNAL(returnPressed()), this, SLOT(onProceedLogin()));

	// Запрещаем показ виртуальной клавиатуры для поля ввода
	lePassword->setAttribute(Qt::WA_InputMethodEnabled, false);
}

//------------------------------------------------------------------------
bool MainServiceWindow::initialize()
{
	connect(twServiceScreens, SIGNAL(currentChanged(int)), SLOT(onCurrentPageChanged(int)));

	// Обновим состояние диспенсера
	mBackend->saveDispenserUnitState();
	
	mBackend->getTerminalInfo(mTerminalInfo);
	lbTerminalNumber->setText(tr("#terminal_number") + mTerminalInfo[CServiceTags::TerminalNumber].toString());
	lbVersion->setText(tr("#software_version") + mTerminalInfo[CServiceTags::SoftwareVersion].toString());

	btnToggleLock->setText(mTerminalInfo[CServiceTags::TerminalLocked].toBool() ?
		tr("#title_unlock") : tr("#title_lock"));

	if (mBackend->isAuthorizationEnabled() && mBackend->hasAnyPassword())
	{
		swPages->setCurrentWidget(wPasswordPage);
		lbStatusMessage->clear();
		lePassword->clear();
		lePassword->setFocus();
		mIdleTimer.setInterval(CMainServiceWindow::PasswordIdleTimeout);
		mIdleTimer.start();
	}
	else
	{
		applyConfiguration();
	}

	onDateTimeRefresh();
	mDateTimeTimer.start();

	return true;
}

//------------------------------------------------------------------------
void MainServiceWindow::shutdown()
{
	//TODO FIX CRASH
	
	disconnect(twServiceScreens, SIGNAL(currentChanged(int)), this, SLOT(onCurrentPageChanged(int)));

	IServiceWindow * current = dynamic_cast<IServiceWindow *>(twServiceScreens->widget(mCurrentPageIndex));
	if (current)
	{
		current->deactivate();
	}

	foreach(IServiceWindow * window, mServiceWindowList)
	{
		window->shutdown();
		delete window;
	}

	twServiceScreens->clear();
	mServiceWindowList.clear();
}

//------------------------------------------------------------------------
void MainServiceWindow::onIdleTimeout()
{
	mBackend->toLog("Timeout Login in service_menu scenario.");

	closeMenu(true);
}

//------------------------------------------------------------------------
void MainServiceWindow::onDateTimeRefresh()
{
	lbCurrentDate->setText(QDateTime::currentDateTime().toString("dd/MM/yyyy, hh:mm"));
}

//------------------------------------------------------------------------
void MainServiceWindow::applyConfiguration()
{
	swPages->setCurrentWidget(wServiceMenuPage);
	applyAccessRights();
}

//------------------------------------------------------------------------
bool MainServiceWindow::applyAccessRights()
{
	ServiceMenuBackend::TAccessRights rights = mBackend->getAccessRights();

	auto addServiceWindow = [&](IServiceWindow * aServiceWindow, const QString & aTitle) 
	{
		mServiceWindowList << aServiceWindow;

		if (aServiceWindow->initialize())
		{
			QWidget * window = dynamic_cast<QWidget *>(aServiceWindow);

			connectAllAbstractButtons(window);
			twServiceScreens->addTab(window, aTitle);
		}
	};

	// Право на диагностику
	if (rights.contains(ServiceMenuBackend::Diagnostic) || !mBackend->hasAnyPassword())
	{
		addServiceWindow(new DiagnosticsServiceWindow(mBackend, this), tr("#title_diagnostic"));
		addServiceWindow(new LogsServiceWindow(mBackend, this), tr("#title_logs"));
	}

	// Право на инкассацию/диспенсер
	if (rights.contains(ServiceMenuBackend::Encash))
	{
		addServiceWindow(new EncashmentServiceWindow(mBackend, this), tr("#title_encashment"));
	}

	// Права на работу с платежами
	if ((rights.contains(ServiceMenuBackend::ViewPayments) ||
		rights.contains(ServiceMenuBackend::ViewPaymentSummary)))
	{
		addServiceWindow(new PaymentServiceWindow(mBackend, this), tr("#title_payments"));
	}

	// Права на настройку ПО
	if (!mBackend->hasAnyPassword() ||
		rights.contains(ServiceMenuBackend::SetupHardware) ||
		rights.contains(ServiceMenuBackend::SetupNetwork) ||
		rights.contains(ServiceMenuBackend::SetupKeys) ||
		rights.contains(ServiceMenuBackend::Encash)) // а инкасатор может настраивать диспенсер
	{
		addServiceWindow(new SetupServiceWindow(mBackend, this), tr("#title_setup"));
	}

	if (twServiceScreens->count())
	{
		twServiceScreens->setCurrentIndex(0);
		mCurrentPageIndex = 0;
	}

	// Право на остановку ПО
	btnStopApplication->setEnabled(rights.contains(ServiceMenuBackend::StopApplication));

	// Право на блокировку терминала
	btnToggleLock->setEnabled(rights.contains(ServiceMenuBackend::LockTerminal));

	return true;
}

//------------------------------------------------------------------------
void MainServiceWindow::closeMenu(bool aStartIdle)
{
	mIdleTimer.stop();

	mBackend->printDispenserDiffState();
	
	QVariantMap params;
	params["signal"] = "close";

	// После завершения текущего сценария, показываем главное меню.
	params["start_idle"] = aStartIdle;
	mBackend->sendEvent(SDK::PaymentProcessor::EEventType::UpdateScenario, params);
	
	mBackend->toLog("Logout.");
}

//------------------------------------------------------------------------
void MainServiceWindow::connectAllAbstractButtons(QWidget * aParentWidget)
{
	foreach(QAbstractButton * b, aParentWidget->findChildren<QAbstractButton *>())
	{
		connect(b, SIGNAL(clicked()), this, SLOT(onAbstractButtonClicked()));
	}
}

//------------------------------------------------------------------------
void MainServiceWindow::onCurrentPageChanged(int aIndex)
{
	IServiceWindow * prev = dynamic_cast<IServiceWindow *>(twServiceScreens->widget(mCurrentPageIndex));

	if (prev)
	{
		if (!prev->deactivate())
		{
			// Окно не может быть сейчас закрыто.
			twServiceScreens->blockSignals(true);
			twServiceScreens->setCurrentIndex(mCurrentPageIndex);
			twServiceScreens->blockSignals(false);

			return;
		}
	}

	IServiceWindow * next = dynamic_cast<IServiceWindow *>(twServiceScreens->widget(aIndex));

	if (next)
	{
		next->activate();
	}

	mCurrentPageIndex = aIndex;

	QWidget * currentPage = twServiceScreens->widget(mCurrentPageIndex);
	if (currentPage)
	{
		mBackend->toLog(QString("Page activated: %1.").arg(currentPage->objectName()));
	}
}

//------------------------------------------------------------------------
void MainServiceWindow::onAbstractButtonClicked()
{
	QAbstractButton * button = qobject_cast<QAbstractButton *>(sender());

	// Кнопки цифровой клавиатуры не логируем
	if (wNumericPad->isAncestorOf(button))
	{
		return;
	}

	QString message(QString("Button clicked: %1").arg(button->text()));

	QCheckBox * checkBox = qobject_cast<QCheckBox *>(sender());
	if (checkBox)
	{
		checkBox->isChecked() ? message += " (checked)" : message += " (unchecked)";
	}

	message += ".";

	mBackend->toLog(message);
}

//------------------------------------------------------------------------
void MainServiceWindow::onBackspaceClicked()
{
	lePassword->backspace();
	lbStatusMessage->clear();
}

//------------------------------------------------------------------------
void MainServiceWindow::onClearClicked()
{
	lePassword->clear();
	lbStatusMessage->clear();
}

//------------------------------------------------------------------------
void MainServiceWindow::onDigitClicked()
{
	if (sender())
	{
		QVariant digit = sender()->property(CMainServiceWindow::DigitProperty);
		if (digit.isValid())
		{
			lePassword->insert(digit.toString());
			lbStatusMessage->clear();
		}
	}
}

//------------------------------------------------------------------------
void MainServiceWindow::onProceedLogin()
{
	if (mBackend->authorize(lePassword->text()))
	{
		mIdleTimer.stop();

		applyConfiguration();
		mBackend->toLog(QString("%1 has logged in.").arg(mBackend->getUserRole()));

		mBackend->saveDispenserUnitState();
	}
	else
	{
		mIdleTimer.start();

		lePassword->clear();
		lbStatusMessage->setText(tr("#error_auth_failed"));
		lePassword->setFocus();
		mBackend->toLog("Authentication failed.");
	}
}

//------------------------------------------------------------------------
void MainServiceWindow::onCancelAuthorization()
{
	mIdleTimer.stop();

	QVariantMap params;
	params["signal"] = "close";
	mBackend->sendEvent(SDK::PaymentProcessor::EEventType::UpdateScenario, params);
}

//------------------------------------------------------------------------
void MainServiceWindow::onRebootApplication()
{
	if (closeServiceMenu(false, tr("#question_reboot_software")))
	{
		mBackend->sendEvent(SDK::PaymentProcessor::EEventType::Restart);
	}
}

//------------------------------------------------------------------------
void MainServiceWindow::onRebootTerminal()
{
	if (closeServiceMenu(false, tr("#question_reboot_terminal")))
	{
		mBackend->sendEvent(SDK::PaymentProcessor::EEventType::Reboot);
	}
}

//------------------------------------------------------------------------
void MainServiceWindow::onToggleLock()
{
	bool isLocked = mTerminalInfo[CServiceTags::TerminalLocked].toBool();
	if (closeServiceMenu(false, isLocked ? tr("#question_unblock_terminal") : tr("#question_block_terminal")))
	{
		mBackend->sendEvent(isLocked ? SDK::PaymentProcessor::EEventType::TerminalUnlock : SDK::PaymentProcessor::EEventType::TerminalLock);
	}
}

//------------------------------------------------------------------------
void MainServiceWindow::onStopApplication()
{
	if (closeServiceMenu(false, tr("#question_stop_terminal")))
	{
		mBackend->sendEvent(SDK::PaymentProcessor::EEventType::StopSoftware);
	}
}

//------------------------------------------------------------------------
bool MainServiceWindow::closeServiceMenu(bool aExitByNotify, const QString & aMessage, bool aStartIdle)
{
	static QTime lastExitQuestionTime;

	if (aExitByNotify)
	{
		if (!lastExitQuestionTime.isNull() && lastExitQuestionTime.secsTo(QTime::currentTime()) < CMainServiceWindow::ExitQuestionTimeout)
		{
			return false;
		}
	}

	if (GUI::MessageBox::question(aMessage))
	{
		IServiceWindow * window = dynamic_cast<IServiceWindow *>(twServiceScreens->widget(mCurrentPageIndex));
		if (window)
		{
			window->deactivate();
		}

		lastExitQuestionTime = QTime::currentTime();
		closeMenu(aStartIdle);
		return true;
	}
	else if (aExitByNotify)
	{
		lastExitQuestionTime = QTime::currentTime();
	}

	return false;
}

//------------------------------------------------------------------------
void MainServiceWindow::onCloseServiceMenu()
{
	if (swPages->currentWidget() == wPasswordPage)
	{
		return;
	}

	closeServiceMenu(false, tr("#question_leave_service_menu"), true);
}

//------------------------------------------------------------------------
