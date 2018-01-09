/* @file Модуль управления сторожевым сервисом через сокет. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QSignalMapper>
#include <QtGui/QMenu>
#include <QtGui/QSystemTrayIcon>
#include <Common/QtHeadersEnd.h>

// Modules
#include <WatchServiceClient/IWatchServiceClient.h>
#include <Common/ILog.h>

//----------------------------------------------------------------------------
class WatchServiceController : public QObject
{
	Q_OBJECT

	// Последняя команда выполненная пользователем 
	enum LastCommand
	{
		Unknown,
		Start,
		Stop
	};

public:
	WatchServiceController();
	~WatchServiceController();

private:
	void onTrayIconContextMenu();

	ILog * getLog();

private slots:
	// Попытка соединения со сторожевым сервисом.
	void onCheck();

	// Закрыто соединение со сторожевым сервисом.
	void onDisconnected();

	// Получение команды на закрытие
	void onCloseCommandReceived();

	void onTrayIconActivated(QSystemTrayIcon::ActivationReason aReason);

	void onStartServiceClicked(QString aName);
	void onStopServiceClicked();
	void onCloseIconClicked();

private:
	QSharedPointer<IWatchServiceClient> mClient;
	
	LastCommand mLastCommand;

	QTimer mTimer;

	QSystemTrayIcon mIcon;
	QMenu mMenu;

	QSignalMapper * mSignalMapper;
	QList<QAction *> mStartServiceActions;
	QAction * mStopServiceAction;
	QAction * mCloseTrayIconAction;
};

//----------------------------------------------------------------------------
