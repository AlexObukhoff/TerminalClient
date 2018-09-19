/* @file Модуль управления сторожевым сервисом через сокет. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/Application.h>
#include <WatchServiceClient/Constants.h>

// Project
#include "WatchServiceController.h"

//----------------------------------------------------------------------------
namespace CWatchServiceController
{
	const int CheckTimeout = 3 * 1000;
}

//----------------------------------------------------------------------------
WatchServiceController::WatchServiceController()
	: mClient(createWatchServiceClient(CWatchService::Modules::WatchServiceController)), mLastCommand(Unknown)
{
	connect(&mTimer, SIGNAL(timeout()), SLOT(onCheck()));


	mClient->subscribeOnDisconnected(this);
	mClient->subscribeOnCloseCommandReceived(this);

	mTimer.setInterval(CWatchServiceController::CheckTimeout);
	mTimer.start();

	mSignalMapper = new QSignalMapper(this);

	{
		auto action = mMenu.addAction(QIcon(":/icons/play.ico"), tr("#start_service_menu"));
		connect(action, SIGNAL(triggered(bool)), mSignalMapper, SLOT(map()));
		mSignalMapper->setMapping(action, QString("-start_scenario=service_menu"));
		mStartServiceActions << action;

		action = mMenu.addAction(QIcon(":/icons/play.ico"), tr("#start_first_setup"));
		connect(action, SIGNAL(triggered(bool)), mSignalMapper, SLOT(map()));
		mSignalMapper->setMapping(action, QString("-start_scenario=first_setup"));
		mStartServiceActions << action;

		mMenu.addSeparator();
	}

	auto action = mMenu.addAction(QIcon(":/icons/play.ico"), tr("#start_service"));
	connect(action, SIGNAL(triggered(bool)), mSignalMapper, SLOT(map()));

	mSignalMapper->setMapping(action, QString("--disable-web-security"));	
	mStartServiceActions << action;

	mStopServiceAction = mMenu.addAction(QIcon(":/icons/stop.ico"), tr("#stop_service"));
	mMenu.addSeparator();
	mCloseTrayIconAction = mMenu.addAction(tr("#close"));


	connect(mSignalMapper, SIGNAL(mapped(QString)), SLOT(onStartServiceClicked(QString)));
	connect(mStopServiceAction, SIGNAL(triggered(bool)), SLOT(onStopServiceClicked()));
	connect(mCloseTrayIconAction, SIGNAL(triggered(bool)), SLOT(onCloseIconClicked()));

	connect(&mIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));

	mIcon.setContextMenu(&mMenu);
	mIcon.setIcon(QIcon(":/icons/tray.png"));
	mIcon.show();

	LOG(getLog(), LogLevel::Normal, "WatchServiceController started.");
}

//----------------------------------------------------------------------------
WatchServiceController::~WatchServiceController()
{
	LOG(getLog(), LogLevel::Normal, "WatchServiceController stopped.");
}

//----------------------------------------------------------------------------
ILog * WatchServiceController::getLog()
{
	if (BasicApplication::getInstance())
	{
		return BasicApplication::getInstance()->getLog();
	}

	return nullptr;
}

//----------------------------------------------------------------------------
void WatchServiceController::onCheck()
{
	if (!mClient->isConnected())
	{
		mLastCommand = Unknown;

		mClient->start();
	}

	if (mClient->isConnected())
	{
		mIcon.setIcon(QIcon(":/icons/tray.png"));
		foreach(auto action, mStartServiceActions)
		{
			action->setEnabled(false);
		}
		mStopServiceAction->setEnabled(true);
	}
	else
	{
		mIcon.setIcon(QIcon(":/icons/tray_stopped.png"));
		foreach(auto action, mStartServiceActions)
		{
			action->setEnabled(true);
		}
		mStopServiceAction->setEnabled(false);
	}

	mIcon.show();
}

//----------------------------------------------------------------------------
void WatchServiceController::onDisconnected()
{
	onCheck();
}

//----------------------------------------------------------------------------
void WatchServiceController::onCloseCommandReceived()
{
	if (mLastCommand != Stop)
	{
		LOG(getLog(), LogLevel::Normal, "Close tray by command from watch service.");

		QCoreApplication::instance()->quit();
	}
	else
	{
		LOG(getLog(), LogLevel::Normal, "Ignore close command, because I initiate it.");
	}
}

//----------------------------------------------------------------------------
void WatchServiceController::onStartServiceClicked(const QString & aArguments)
{
	LOG(getLog(), LogLevel::Normal, QString("User say: start service. %1").arg(aArguments));

	mLastCommand = Start;

	if (!mClient->isConnected())
	{
		QString path = QDir::cleanPath(QDir::toNativeSeparators(
			BasicApplication::getInstance()->getWorkingDirectory() + QDir::separator() + "guard.exe"));

		QStringList parameters;

		if (!aArguments.isEmpty())
		{
			parameters << QString("-client_options=%1").arg(aArguments);
		}

		QProcess::startDetached(path, parameters, BasicApplication::getInstance()->getWorkingDirectory());
	}
	else
	{
		mClient->restartService(QStringList());
	}
}

//----------------------------------------------------------------------------
void WatchServiceController::onStopServiceClicked()
{
	LOG(getLog(), LogLevel::Normal, "User say: stop service.");

	mLastCommand = Stop;

	if (mClient->isConnected())
	{
		mClient->stopService();
	}
}

//----------------------------------------------------------------------------
void WatchServiceController::onCloseIconClicked()
{
	QMessageBox msgBox(tr("#exit"),
		tr("#confirm_close_trayicon"),
		QMessageBox::Question,
		0,
		0,
		0,
		0,
		Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);

	msgBox.setWindowIcon(QIcon(":/icons/tray.png"));

	QPushButton * yesButton = msgBox.addButton(tr("#yes"), QMessageBox::YesRole);
	msgBox.addButton(tr("#no"), QMessageBox::NoRole);

	msgBox.exec();
	if (msgBox.clickedButton() == yesButton)
	{
		QCoreApplication::instance()->quit();
	}
}

//----------------------------------------------------------------------------
void WatchServiceController::onTrayIconActivated(QSystemTrayIcon::ActivationReason aReason)
{
	onCheck();

	// Вызываем контекстнео меню по нажатию левой кнопки мыши
	if (aReason == QSystemTrayIcon::Trigger)
	{
		mMenu.popup(QCursor::pos());
		mMenu.activateWindow();
	}
}

//----------------------------------------------------------------------------
