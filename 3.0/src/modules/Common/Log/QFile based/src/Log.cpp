/* @file Реализация статических методов интерфейса ILog. */

#include <Common/ILog.h>
#include "LogManager.h"

//---------------------------------------------------------------------------
extern LogManager gLogManager;

//---------------------------------------------------------------------------
ILog * ILog::getInstance()
{
	return getInstance("Default");
}

//---------------------------------------------------------------------------
ILog * ILog::getInstance(const QString & aName)
{
	return getInstance(aName, LogType::File);
}

//---------------------------------------------------------------------------
ILog * ILog::getInstance(const QString & aName, LogType::Enum aType)
{
	return gLogManager.getLog(aName, aType);
}

//---------------------------------------------------------------------------
void ILog::logRotateAll()
{
	gLogManager.logRotateAll();
}

//---------------------------------------------------------------------------
void ILog::setGlobalLevel(LogLevel::Enum aMaxLogLevel)
{
	gLogManager.setGlobalLevel(aMaxLogLevel);
}

//---------------------------------------------------------------------------
