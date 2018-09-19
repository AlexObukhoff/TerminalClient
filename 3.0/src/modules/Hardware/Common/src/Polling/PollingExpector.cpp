/* @file Класс-expector для ожидания состояния. */

#include "Hardware/Common/MutexLocker.h"
#include "PollingExpector.h"

// инстанцируем одну из специализаций по умолчанию
template bool PollingExpector::wait<void>(TVoidMethod aOnPoll, TBoolMethod aCondition, TBoolMethod aErrorCondition, const SWaitingData & aWaitingData);
template bool PollingExpector::wait<void>(TVoidMethod aOnPoll, TBoolMethod aCondition, TBoolMethod aErrorCondition, int aPollingInterval, int aTimeout, bool aPollingSensible);
template bool PollingExpector::wait<void>(TVoidMethod aOnPoll, TBoolMethod aCondition, const SWaitingData & aWaitingData);
template bool PollingExpector::wait<void>(TVoidMethod aOnPoll, TBoolMethod aCondition, int aPollingInterval, int aTimeout, bool aPollingSensible);
template bool PollingExpector::wait<bool>(TBoolMethod aOnPoll, TBoolMethod aCondition, int aPollingInterval, int aTimeout, bool aPollingSensible);

//--------------------------------------------------------------------------------
ExpectorWorkingThread::ExpectorWorkingThread()
{
	moveToThread(this);
	mPolling.moveToThread(this);
	mOwner = QThread::currentThread();
	mPollingSensible = false;

	connect(&mPolling, SIGNAL(timeout()), SLOT(onPoll()), Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------
void ExpectorWorkingThread::run()
{
	MutexLocker::setMatchedThread(mOwner, currentThread());

	QThread::exec();

	MutexLocker::clearMatchedThread(mOwner);
}

//--------------------------------------------------------------------------------
void ExpectorWorkingThread::process(TBoolMethod aOnPoll, TBoolMethod aCondition, TBoolMethod aErrorCondition, int aPollingInterval, bool aPollingSensible)
{
	mOnPoll = aOnPoll;
	mPollingSensible = aPollingSensible;
	mCondition = aCondition;
	mErrorCondition = aErrorCondition;
	mPolling.setInterval(aPollingInterval);

	if (!isRunning())
	{
		start();
	}

	QMetaObject::invokeMethod(&mPolling, "start", Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------
void ExpectorWorkingThread::onPoll()
{
	bool result =  !mOnPoll || mOnPoll();

	if ((!result && mPollingSensible) || (mErrorCondition && mErrorCondition()))
	{
		emit finished(false);

		mPolling.stop();
	}

	if (mCondition())
	{
		emit finished(true);

		mPolling.stop();
	}
}

//--------------------------------------------------------------------------------
PollingExpector::PollingExpector() : mResult(false)
{
	moveToThread(&mWorkingThread);
	connect(&mWorkingThread, SIGNAL(finished(bool)), SLOT(onFinished(bool)));
}

//--------------------------------------------------------------------------------
bool PollingExpector::wait(TBoolMethod aCondition, int aPollingInterval, int aTimeout, bool aPollingSensible)
{
	return wait(aCondition, SWaitingData(aPollingInterval, aTimeout, aPollingSensible));
}

//--------------------------------------------------------------------------------
bool PollingExpector::wait(TBoolMethod aCondition, const SWaitingData & aWaitingData)
{
	return wait(TBoolMethod(), aCondition, aWaitingData);
}

//--------------------------------------------------------------------------------
template <class T>
bool PollingExpector::wait(std::function<T()> aOnPoll, TBoolMethod aCondition, int aPollingInterval, int aTimeout, bool aPollingSensible)
{
	return wait(aOnPoll, aCondition, SWaitingData(aPollingInterval, aTimeout, aPollingSensible));
}

//--------------------------------------------------------------------------------
template <class T>
bool PollingExpector::wait(std::function<T()> aOnPoll, TBoolMethod aCondition, const SWaitingData & aWaitingData)
{
	return wait(aOnPoll, aCondition, TBoolMethod(), aWaitingData);
}

//--------------------------------------------------------------------------------
template <class T>
bool PollingExpector::wait(std::function<T()> aOnPoll, TBoolMethod aCondition, TBoolMethod aErrorCondition, int aPollingInterval, int aTimeout, bool aPollingSensible)
{
	return wait(aOnPoll, aCondition, aErrorCondition, SWaitingData(aPollingInterval, aTimeout, aPollingSensible));
}

//--------------------------------------------------------------------------------
template <class T>
bool PollingExpector::wait(std::function<T()> aOnPoll, TBoolMethod aCondition, TBoolMethod aErrorCondition, const SWaitingData & aWaitingData)
{
	return wait<bool>([&] () -> bool { aOnPoll(); return true; }, aCondition, aErrorCondition, aWaitingData);
}

//--------------------------------------------------------------------------------
template <>
bool PollingExpector::wait(std::function<bool()> aOnPoll, TBoolMethod aCondition, TBoolMethod aErrorCondition, const SWaitingData & aWaitingData)
{
	mResult = false;

	if (!aCondition)
	{
		return false;
	}

	bool OK = aCondition();
	bool error = aErrorCondition && aErrorCondition();

	if (OK || error)
	{
		return OK && !error;
	}

	if (aOnPoll && !aOnPoll() && aWaitingData.pollingSensible)
	{
		return false;
	}

	OK = aCondition();
	error = aErrorCondition && aErrorCondition();

	if (OK || error)
	{
		return OK && !error;
	}

	{
		QMutexLocker locker(&mGuard);

		mWorkingThread.process(aOnPoll, aCondition, aErrorCondition, aWaitingData.pollingInterval, aWaitingData.pollingSensible);
		mWaitCondition.wait(&mGuard, aWaitingData.timeout);
	}

	mWorkingThread.quit();
	mWorkingThread.wait();

	return mResult;
}

//--------------------------------------------------------------------------------
void PollingExpector::onFinished(bool aSuccess)
{
	mWaitCondition.wakeAll();
	mResult = aSuccess;
}

//--------------------------------------------------------------------------------
