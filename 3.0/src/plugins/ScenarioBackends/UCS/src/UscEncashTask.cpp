/* @file Реализация задачи инкассации терминала по расписанию. */

// Модули


// Проект
#include "UscEncashTask.h"

namespace Ucs
{

//---------------------------------------------------------------------------
UscEncashTask::UscEncashTask(const QString & aName, const QString & aLogName, const QString & aParams)
	: ITask(aName, aLogName, aParams)
{
}

//---------------------------------------------------------------------------
void UscEncashTask::execute()
{
	emit finished(mName, true);
}

//---------------------------------------------------------------------------
bool UscEncashTask::subscribeOnComplete(QObject * aReceiver, const char * aSlot)
{
	return connect(this, SIGNAL(finished(const QString &, bool)), aReceiver, aSlot);
}

}

//---------------------------------------------------------------------------

