/* @file Реализация задачи обновления контента удалённых сервисов. */

// Модули
#include <System/IApplication.h>
#include <Common/Application.h>

// Проект
#include "Services/RemoteService.h"
#include "UpdateRemoteContent.h"

//---------------------------------------------------------------------------
UpdateRemoteContent::UpdateRemoteContent(const QString & aName, const QString & aLogName, const QString & aParams) 
	: ITask(aName, aLogName, aParams)
{
}

//---------------------------------------------------------------------------
void UpdateRemoteContent::execute()
{
	IApplication * app = dynamic_cast<IApplication *>(BasicApplication::getInstance());

	app->getCore()->getRemoteService()->updateContent();

	emit finished(mName, true);
}

//---------------------------------------------------------------------------
bool UpdateRemoteContent::subscribeOnComplete(QObject * aReceiver, const char * aSlot)
{
	return connect(this, SIGNAL(finished(const QString &, bool)), aReceiver, aSlot);
}

//---------------------------------------------------------------------------

