/* @file Различные средства отладки. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// Common
#include <Common/ILog.h>

//---------------------------------------------------------------------------
class TraceLogger
{
public:
	TraceLogger(const char * aFileName, const char * aFuncName, int aLineNumber)
	{
		mFileName = aFileName;
		mFuncName = aFuncName;
		
		ILog::getInstance(mLogName)->write(LogLevel::Normal, QString("%1Entering %2() (%3:%4)").arg(QString(" ").repeated(mIndent)).arg(mFuncName).arg(mFileName).arg(aLineNumber));
		
		mIndent++;
	}

	~TraceLogger()
	{
		mIndent--;

		ILog::getInstance(mLogName)->write(LogLevel::Normal, QString("%1Leaving  %2() (%3)").arg(QString(" ").repeated(mIndent)).arg(mFuncName).arg(mFileName));
	}

private:
	const char * mFileName;
	const char * mFuncName;
	
private:
	static int mIndent;
	static const char * mLogName;
};

#define LOG_TRACE() TraceLogger traceLogger(__FILE__, __FUNCTION__, __LINE__)

#define ENABLE_TRACE_LOGGER(aLogName) \
	int TraceLogger::mIndent = 0; \
	const char * TraceLogger::mLogName = aLogName;

