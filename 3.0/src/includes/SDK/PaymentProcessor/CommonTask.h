/* @file База для задачи планировщика. */

#pragma once

// Boost
#include <boost/function.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

// Sdk
#include "ITask.h"

//------------------------------------------------------------------------------
namespace SDK
{

//------------------------------------------------------------------------------
namespace PaymentProcessor
{

/// База задачи для планировщика.
class CommonTask : public ITask
{
public:
	typedef boost::function<bool(const ITask::TContext &)> TCondition;
	typedef boost::function<void()> TMethod;

	CommonTask(TCondition aCondition, TMethod aMethod, bool aIsThread = false)
		: mCondition(aCondition),
		  mMethod(aMethod),
		  mIsThread(aIsThread)
	{
	}

	virtual ~CommonTask()
	{
	}

	/// ITask: Предикат возвращает true, если задача может быть выполнена в данный момент.
	virtual bool isReady(TContext & aContext)
	{
		aContext[TaskContext::LastActivation] = mLastActivation;

		bool result = mCondition(aContext);

		aContext.remove(TaskContext::LastActivation);

		return result;
	}

	/// ITask: Возвращает true, если задача должна выполняться в отдельном потоке.
	virtual bool isThread() const
	{
		return mIsThread;
	}

	/// ITask: Рабочая процедура задачи.
	virtual void run()
	{
		mLastActivation = QDateTime::currentDateTime();

		mMethod();
	}

private:
	TCondition mCondition;
	TMethod    mMethod;
	bool       mIsThread;

	QDateTime  mLastActivation;
};

//------------------------------------------------------------------------------
} // PaymentProcessor

//------------------------------------------------------------------------------
} // SDK

