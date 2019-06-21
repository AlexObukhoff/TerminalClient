/* @file ќбертка над подсистемой Windows BITS. */

// Project
#include "WindowsBITS.h"
#include "WindowsBITS_p.h"


namespace CBITS
{

//---------------------------------------------------------------------------
CopyManager::CopyManager(ILog * aLog) : ILogable(aLog), mJobsCount(0), mPriority(CBITS::HIGH)
{
	mCopyManager = QSharedPointer<CopyManager_p>(new CopyManager_p(aLog));
}

//---------------------------------------------------------------------------
CopyManager::~CopyManager()
{
}

//---------------------------------------------------------------------------
void CopyManager::shutdown()
{
	mCopyManager.clear();
}

//---------------------------------------------------------------------------
bool CopyManager::isReady() const
{
	return mCopyManager && mCopyManager->isReady();
}

//---------------------------------------------------------------------------
QMap<QString, SJob> CopyManager::getJobs(const QString & aFilter)
{
	QMap<QString, SJob> result;

	if (isReady())
	{
		auto allJobs = mCopyManager->getJobs();

		foreach(auto key, QStringList(allJobs.keys()).filter(aFilter, Qt::CaseInsensitive))
		{
			result.insert(key, allJobs.value(key));
		}
	}

	return result;
}

//---------------------------------------------------------------------------
bool CopyManager::createJob(const QString & aName, SJob & aJob, int aPriority)
{
	if (isReady())
	{
		mPriority = aPriority;
		return mCopyManager->createJob(makeJobName(aName), aJob, aPriority);
	}

	return false;
}

//---------------------------------------------------------------------------
bool CopyManager::setNotify(const QString & aApplicationPath, const QString & aParamaters)
{
	mNotifyApplication = aApplicationPath;
	mNotifyParameters = aParamaters;
	return true;
}

//---------------------------------------------------------------------------
bool CopyManager::addTask(const QUrl & aUrl, const QString & aFileName)
{
	if (isReady())
	{
		switch (mCopyManager->addTask(aUrl, aFileName))
		{
		case AddTaskResult::OK:
			return true;
		case AddTaskResult::Error:
			return false;
		case AddTaskResult::JobIsFull:
		{
			if (!internalResume())
			{
				return false;
			}

			toLog(LogLevel::Normal, QString("BITS: Job %1 resumed.").arg(makeJobName()));

			mJobsCount++;
			SJob newJob;
			return
				mCopyManager->createJob(makeJobName(), newJob, mPriority) &&
				addTask(aUrl, aFileName);
		}
		}
	}

	return false;
}

//---------------------------------------------------------------------------
bool CopyManager::openJob(const SJob & aJob)
{
	if (isReady())
	{
		return mCopyManager->openJob(aJob);
	}

	return false;
}

//---------------------------------------------------------------------------
bool CopyManager::internalResume()
{
	return
		mCopyManager->setJobNotify(mNotifyApplication, mNotifyParameters) &&
		mCopyManager->resume();
}

//---------------------------------------------------------------------------
QString CopyManager::makeJobName(const QString & aName /*= QString()*/)
{
	if (!aName.isEmpty())
	{
		mJobName = aName;
	}

	return mJobName + QString("#%1").arg(mJobsCount, 2, 10, QChar('0'));
}

//---------------------------------------------------------------------------
bool CopyManager::resume()
{
	if (isReady() && internalResume())
	{
		toLog(LogLevel::Normal, QString("BITS: Job %1 resumed.").arg(makeJobName()));

		return true;
	}

	return false;
}

//---------------------------------------------------------------------------
bool CopyManager::cancel()
{
	if (isReady())
	{
		return mCopyManager->cancel();
	}

	return false;
}

//---------------------------------------------------------------------------
bool CopyManager::complete()
{
	if (isReady())
	{
		return mCopyManager->complete();
	}

	return false;
}

//---------------------------------------------------------------------------
} // namespace CBITS
