/* @file Класс для выполнения функционала без зависаний. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTime>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/Common/MutexLocker.h"
#include "SafePerformer.h"

//--------------------------------------------------------------------------------
SafePerformerThread::SafePerformerThread(ILog * aLog) : mLog(aLog)
{
	moveToThread(this);
}

//--------------------------------------------------------------------------------
void SafePerformerThread::onTask(const STaskData & aData)
{
	LOG(mLog, LogLevel::Debug, "Task started");

	QTime timer;
	timer.start();

	bool result = aData.task();
	int performingTime = timer.elapsed();

	if (performingTime < aData.timeout)
	{
		LOG(mLog, LogLevel::Debug, QString("Task was performed for %1 ms").arg(performingTime));

		emit finished(result);
	}
	else
	{
		LOG(mLog, LogLevel::Error, QString("Task was performed for %1 ms").arg(performingTime));
		aData.changePerformingTimeout(aData.context, aData.timeout, performingTime);

		if (!aData.forwardingTask._Empty())
		{
			LOG(mLog, LogLevel::Normal, "Going to forwarding task");

			aData.forwardingTask();
		}
	}
}

//--------------------------------------------------------------------------------
SafePerformer::SafePerformer(ILog * aLog) : mLog(aLog), mResult(false)
{
	qRegisterMetaType<STaskData>("STaskData");
}

//--------------------------------------------------------------------------------
ETaskResult::Enum SafePerformer::process(const STaskData & aData)
{
	if (aData.task._Empty())
	{
		return ETaskResult::Invalid;
	}

	SafePerformerThread * workingThread = new SafePerformerThread(mLog);
	connect(workingThread, SIGNAL(finished(bool)), SLOT(onFinished(bool)));

	moveToThread(workingThread);
	workingThread->start();

	mResult = false;

	{
		QMutexLocker locker(&mGuard);

		QMetaObject::invokeMethod(workingThread, "onTask", Qt::QueuedConnection, Q_ARG(const STaskData &, aData));

		if (!mWaitCondition.wait(&mGuard, aData.timeout))
		{
			LOG(mLog, LogLevel::Error, "Cannot perform task during " + QString::number(aData.timeout));
			return ETaskResult::Suspended;
		}

		workingThread->quit();
		workingThread->wait();
	}

	delete workingThread;

	return mResult ? ETaskResult::OK : ETaskResult::Error;
}

//--------------------------------------------------------------------------------
void SafePerformer::onFinished(bool aSuccess)
{
	mResult = aSuccess;

	mWaitCondition.wakeAll();
}

//--------------------------------------------------------------------------------
