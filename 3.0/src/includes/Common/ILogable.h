#pragma once

#include "ILog.h"

class QString;

//---------------------------------------------------------------------------
/// Класс, упрощающий вывод в лог.
class ILogable
{
public:
	ILogable(ILog * aLog = nullptr) : mLog(aLog) {}
	ILogable(const QString & aLogName) : mLog(ILog::getInstance(aLogName)) {}

	virtual ~ILogable() {}

	inline void setLog(ILog * aLog)
	{
		mLog = aLog;
	}

protected:
	inline void toLog(LogLevel::Enum aLevel, const QString & aMessage) const
	{
		if (mLog)
		{
			mLog->write(aLevel, aMessage);
		}
		else
		{
			qCritical("Log pointer is empty. Message:%s.", aMessage.toLocal8Bit().data());
		}
	}

	inline ILog * getLog() const
	{
		return mLog;
	}

private:
	ILog * mLog;
};

//---------------------------------------------------------------------------
