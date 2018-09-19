/* @file Помощник по логу. */

#pragma once

// Common
#include <Common/ILog.h>

//--------------------------------------------------------------------------------
class DeviceLogManager
{
public:
	DeviceLogManager(): mLog(nullptr) {}
	DeviceLogManager(ILog * aLog): mLog(aLog) {}

	/// Логгировать.
	void toLog(LogLevel::Enum aLevel, const QString & aMessage) const
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

	/// Установить лог.
	void setLog(ILog * aLog)
	{
		mLog = aLog;
	}

protected:
	/// Лог.
	ILog * mLog;
};

//--------------------------------------------------------------------------------
