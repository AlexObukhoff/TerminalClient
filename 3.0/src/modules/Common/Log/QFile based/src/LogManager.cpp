/* @file Лог-менеджер. */

#include "SimpleLog.h"
#include "LogManager.h"

//---------------------------------------------------------------------------
LogManager gLogManager;

//---------------------------------------------------------------------------
LogManager::LogManager()
	: mMaxLogLevel(LogLevel::Normal)
{
#if defined(_DEBUG) || defined(DEBUG_INFO)
	mMaxLogLevel = LogLevel::Debug;
#endif // _DEBUG || DEBUG_INFO
}

//---------------------------------------------------------------------------
LogManager::~LogManager()
{
	QMutexLocker lock(&mMutex);

	while (!mLogs.isEmpty())
	{
		auto log = mLogs.take(mLogs.keys().first());
		
		log.reset();
	}
}

//---------------------------------------------------------------------------
ILog * LogManager::getLog(const QString & aName, LogType::Enum aType)
{
	QString name = QString("%1%2").arg(aName).arg(aType);

	QMutexLocker lock(&mMutex);

	if (mLogs.contains(name))
	{
		return mLogs.value(name).get();
	}

	std::shared_ptr<ILog> newlog(new SimpleLog(aName, aType, mMaxLogLevel));
	mLogs.insert(name, newlog);

	return newlog.get();
}

//---------------------------------------------------------------------------
void LogManager::logRotateAll()
{
	QMutexLocker lock(&mMutex);

	foreach (auto log, mLogs.values())
	{
		log->logRotate();
	}
}

//---------------------------------------------------------------------------
void LogManager::setGlobalLevel(LogLevel::Enum aMaxLogLevel)
{
	mMaxLogLevel = aMaxLogLevel;
}

//---------------------------------------------------------------------------
