/* @file Реализация задачи архивации журнальных файлов. */

// Модули
#include <System/IApplication.h>
#include <Common/Application.h>

// Проект
#include "Services/TerminalService.h"
#include "LogRotate.h"

//---------------------------------------------------------------------------
LogRotate::LogRotate(const QString & aName, const QString & aLogName, const QString & aParams) 
	: ITask(aName, aLogName, aParams)
{
}

//---------------------------------------------------------------------------
void LogRotate::execute()
{
	IApplication * app = dynamic_cast<IApplication *>(BasicApplication::getInstance());

	if (app)
	{
		auto terminalService = dynamic_cast<TerminalService *>(app->getCore()->getTerminalService());

		if (terminalService)
		{
			terminalService->getClient()->execute("close_logs");
		}
	}

	ILog::logRotateAll();

	emit finished(mName, true);
}

//---------------------------------------------------------------------------
bool LogRotate::subscribeOnComplete(QObject * aReceiver, const char * aSlot)
{
	return connect(this, SIGNAL(finished(const QString &, bool)), aReceiver, aSlot);
}

//---------------------------------------------------------------------------

