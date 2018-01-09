/* @file Реализация задачи запуска модуля обновления. */

// SDK
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>

// Modules
#include <System/IApplication.h>
#include <Common/Application.h>

// Project
#include "RunUpdater.h"
#include "Services/RemoteService.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
RunUpdater::RunUpdater(const QString & aName, const QString & aLogName, const QString & aParams) 
	: ITask(aName, aLogName, aParams),
	ILogable(aLogName),
	mParams(aParams)
{
}

//---------------------------------------------------------------------------
RunUpdater::~RunUpdater()
{
}

//---------------------------------------------------------------------------
void RunUpdater::execute()
{
	IApplication * app = dynamic_cast<IApplication *>(BasicApplication::getInstance());

	PPSDK::ICore * core = app->getCore();
	PPSDK::TerminalSettings * terminalSettings = static_cast<PPSDK::TerminalSettings *>(core->getSettingsService()->
		getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

	auto urls = terminalSettings->getUpdaterUrls();
	if (urls.size() != 2 || urls[0].isEmpty() || urls[1].isEmpty())
	{
		toLog(LogLevel::Error, "Invalid configuration updater URLs");
		
		emit finished(mName, false);
		return;
	}

	PPSDK::IRemoteService * monitoring = core->getRemoteService();

	int cmdId = monitoring->registerUpdateCommand(PPSDK::IRemoteService::Update, urls[0], urls[1], mParams);

	if (cmdId > 0)
	{
		toLog(LogLevel::Normal, QString("Register update command %1 in monitioring service. Components '%2'").arg(cmdId).arg(mParams));
	}
	else
	{
		toLog(LogLevel::Warning, QString("Error register update command in monitioring service. Components '%1'").arg(mParams));
	}

	emit finished(mName, cmdId > 0);
}

//---------------------------------------------------------------------------
bool RunUpdater::subscribeOnComplete(QObject * aReceiver, const char * aSlot)
{
	return connect(this, SIGNAL(finished(const QString &, bool)), aReceiver, aSlot);
}

//---------------------------------------------------------------------------

