/* @file Реализация сторожевого сервиса как обычного приложения. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QSingleApplication/QtSingleApplication>
#include <QBreakpadHandler.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/Application.h>
#include <Common/Version.h>

// Project
#include "WatchService.h"

//----------------------------------------------------------------------------
namespace CWatchService
{
	const QString Name = "WatchService";
}

//----------------------------------------------------------------------------
void qtMessageHandler(QtMsgType /*aType*/, const char * aMessage)
{
	static ILog * log = ILog::getInstance("QtMessages");

	log->write(LogLevel::Normal, QString::fromLatin1(aMessage));
}

//----------------------------------------------------------------------------
int main(int aArgc, char * aArgv[])
{
	BasicQtApplication<QtSingleApplication> application(CWatchService::Name, Cyberplat::getVersion(), aArgc, aArgv);

	// Если сервис уже запущен выходим.
	if (application.getQtApplication().sendMessage("Instance!!!"))
	{
		LOG(application.getLog(), LogLevel::Warning, "Another instance is already running.");

		return 0;
	}

	QBreakpadInstance.setDumpPath(application.getWorkingDirectory() + "/logs/");

	// Перенаправляем логи
	ILog * mainLog = application.getLog();
	ILog::getInstance("ConfigManager")->setDestination(mainLog->getDestination());
	ILog::getInstance("CryptEngine")->setDestination(mainLog->getDestination());
	ILog::getInstance("QtMessages")->setDestination(mainLog->getDestination());

	application.getQtApplication().initialize();
	application.getQtApplication().setQuitOnLastWindowClosed(false);

	qInstallMsgHandler(qtMessageHandler);

	WatchService service;

	int result = application.exec();

	qInstallMsgHandler(0);

	return result;
}

//----------------------------------------------------------------------------

