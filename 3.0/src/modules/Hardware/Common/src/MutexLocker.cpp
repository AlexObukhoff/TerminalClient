/* @file Мьютекс-локер с возможностью ограничения синхронизации определенного потока. */

// Project
#include "MutexLocker.h"

MutexLocker::TMatchedThreads MutexLocker::mMatchedThreads;
QMutex MutexLocker::mResourceMutex(QMutex::Recursive);
MutexLocker::TThreadsLocked MutexLocker::mThreadsLocked;

//--------------------------------------------------------------------------------
MutexLocker::MutexLocker(QMutex * aMutex) :
	mMutex(nullptr)
{
	if (aMutex)
	{
		mMutex = aMutex;
		QThread * matchedThread = nullptr;

		{
			QMutexLocker locker(&mResourceMutex);

			if (!mThreadsLocked.contains(mMutex))
			{
				mThreadsLocked.insert(mMutex, TLocksCounter(nullptr, 0));
			}
			else if (mMatchedThreads.contains(mThreadsLocked[mMutex].first))
			{
				matchedThread = mMatchedThreads[mThreadsLocked[mMutex].first];
			}
		}

		QThread * currentThread = QThread::currentThread();

		if (matchedThread != currentThread)
		{
			mMutex->lockInline();

			{
				QMutexLocker locker(&mResourceMutex);

				mThreadsLocked[mMutex].first = currentThread;
				mThreadsLocked[mMutex].second++;
			}
		}
	}
}

//--------------------------------------------------------------------------------
MutexLocker::~MutexLocker()
{
	if (mMutex)
	{
		QMutexLocker locker(&mResourceMutex);

		QThread * currentThread = QThread::currentThread();

		if (mThreadsLocked.contains(mMutex) && (mThreadsLocked[mMutex].first == currentThread))
		{
			mMutex->unlockInline();
			mThreadsLocked[mMutex].second--;

			if (mThreadsLocked[mMutex].second <= 0)
			{
				mThreadsLocked[mMutex].first = nullptr;
			}
		}
	}
}

//--------------------------------------------------------------------------------
void MutexLocker::setMatchedThread(QThread * aOwner, QThread * aMatched)
{
	QMutexLocker locker(&mResourceMutex);

	mMatchedThreads.insert(aOwner, aMatched);
}

//--------------------------------------------------------------------------------
void MutexLocker::clearMatchedThread(QThread * aOwner)
{
	QMutexLocker locker(&mResourceMutex);

	mMatchedThreads.remove(aOwner);
}

//--------------------------------------------------------------------------------
