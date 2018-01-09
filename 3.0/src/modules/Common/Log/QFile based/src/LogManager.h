/* @file Лог-менеджер. */

#pragma once

// STL
#include <memory>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <Common/QtHeadersEnd.h>

// Проект
#include <Common/ILog.h>

//---------------------------------------------------------------------------
class LogManager
{
public:
	LogManager();
	virtual ~LogManager();

	/// Создать или получить инстанс объекта логгирования
	virtual ILog * getLog(const QString & aName, LogType::Enum aType);

	/// Закрыть все журнальные файлы, например при переходе времени на следующие сутки
	virtual void logRotateAll();

	/// Установить уровень логирования для всех логов
	virtual void setGlobalLevel(LogLevel::Enum aMaxLogLevel);

protected:
	QMap<QString, std::shared_ptr<ILog>> mLogs;
	QMutex mMutex;
	LogLevel::Enum mMaxLogLevel;
};

//---------------------------------------------------------------------------
