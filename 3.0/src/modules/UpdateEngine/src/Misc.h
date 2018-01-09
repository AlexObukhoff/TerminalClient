/* @file Вспомогательные функции. */

#pragma once

// Stl
#include <functional>

// Modules
#include <Common/ILog.h>

//---------------------------------------------------------------------------
/// Логирование.
namespace
{
	inline ILog * Log()
	{
		return ILog::getInstance("Updater");
	}

	inline void Log(LogLevel::Enum aLevel, const QString & aMessage)
	{
		Log()->write(aLevel, aMessage);
	}
}

//---------------------------------------------------------------------------
