/* @file Реализация модуля управления сторожевым сервисом через сокет. */

// stl
#include <iostream>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTranslator>
#include <QtWidgets/QApplication>
#include <QSingleApplication/QtSingleApplication>
#include <QBreakpadHandler.h>
#include <Common/QtHeadersEnd.h>

// Модули
#include <Common/Version.h>
#include <Common/ExitCodes.h>
#include <Common/Application.h>

// Проект
#include "WatchServiceController.h"

//----------------------------------------------------------------------------
int main(int aArgc, char * aArgv[])
{
	BasicQtApplication<QtSingleApplication> application("WatchServiceController", Cyberplat::getVersion(), aArgc, aArgv);

	if (application.getQtApplication().sendMessage("Instance!!!"))
	{
		LOG(application.getLog(), LogLevel::Warning, "Another instance is already running.");

		return 0;
	}

	QBreakpadInstance.setDumpPath(application.getWorkingDirectory() + "/logs/");

	// Перенаправляем логи.
	ILog::getInstance(CIMessageQueueClient::DefaultLog)->setDestination(application.getLog()->getName());

	application.getQtApplication().initialize();
	application.getQtApplication().setQuitOnLastWindowClosed(false);

	WatchServiceController controller;

	return application.exec();
}

//----------------------------------------------------------------------------
