/* @file Реализация вспомогательного функционала для сервисов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QThread>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
inline void SafeStopServiceThread(QThread * aThread, int aTimeout, ILog * aLogger)
{
	if (aThread && aThread->isRunning() && aThread != QThread::currentThread())
	{
		aThread->quit();

		if (!aThread->wait(aTimeout))
		{
			LOG(aLogger, LogLevel::Error, QString("Error stop service thread '%1'. Terminate.").arg(aThread->objectName()));
			aThread->terminate();
		}
	}
}