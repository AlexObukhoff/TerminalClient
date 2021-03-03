/* @file Реализация простого логгера в файл. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtCore/QTextStream>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILog.h>

// Проект
#include "LogManager.h"

//---------------------------------------------------------------------------
class SimpleLog;

//---------------------------------------------------------------------------
class DestinationFile
{
	QFile mFile;
	FILE * mStdFile;
	QMutex mMutex;
	QTextStream mLogStream;
	QString mFileName;

protected:
	DestinationFile() : mStdFile(nullptr), mMutex(QMutex::Recursive), mLogStream(&mFile)
	{
		mLogStream.setCodec("utf-8");
	}

	friend class SimpleLog;

public:
	bool open(const QString & aLogPath)
	{
		mFile.close();
		
		if (mStdFile)
		{
			fflush(mStdFile);
			fclose(mStdFile);
		}

		mStdFile = _fsopen(aLogPath.toLocal8Bit().constData(), "ab+", _SH_DENYNO);

		bool isOK = mStdFile && mFile.open(mStdFile, QIODevice::Append | QIODevice::Text);

		mFileName = isOK ? aLogPath : "";

		return isOK;
	}

	bool isOpen() const
	{
		return mFile.isOpen();
	}

	QString fileName() const
	{
		return mFileName;
	}

	void write(const QString & aMessage)
	{
		QMutexLocker locker(&mMutex);

		mLogStream << aMessage;
		mLogStream.flush();
	}
};

//---------------------------------------------------------------------------
typedef QSharedPointer<DestinationFile> DestinationFilePtr;

//---------------------------------------------------------------------------
class SimpleLog : public ILog
{
public:
	SimpleLog(const QString & aName = "Default", LogType::Enum aType = LogType::File, LogLevel::Enum aMaxLogLevel = LogLevel::Normal);
	virtual ~SimpleLog();

	// Методы интерфейса ILog
	/// Возвращает имя экземпляра лога.
	virtual const QString & getName() const;

	/// Возвращает тип вывода данного экземпляра лога.
	virtual LogType::Enum getType() const;

	/// Возвращает направление вывода.
	virtual const QString & getDestination() const;

	/// Устанавливает направление вывода.
	virtual void setDestination(const QString & aDestination);

	/// Устанавливает минимальный уровень, ниже которого логгирование игнорируется.
	virtual void setLevel(LogLevel::Enum aLevel);

	/// Возвращает минимальный уровень, ниже которого логгирование игнорируется.
	virtual LogLevel::Enum getLevel();

	/// Устанавливает уровень отступа для древовидных логов.
	virtual void adjustPadding(int aStep);

	/// Производит запись в лог.
	virtual void write(LogLevel::Enum aLevel, const QString & aMessage);

	/// Производит запись в лог c форматированием данных.
	virtual void write(LogLevel::Enum aLevel, const QString & aMessage, const QByteArray & aData);

	/// Принудительно закрыть журнал. Функция write заново его откроет.
	virtual void logRotate();

protected:
	virtual bool init();
	virtual bool isInitiated();
	virtual void safeWrite(LogLevel::Enum aLevel, const QString & aMessage);

private:
	virtual void writeHeader();

private:
	bool mInitOk;
	LogLevel::Enum mMaxLogLevel;

	QString mName;
	QString mDestination;
	LogType::Enum mType;
	int mPadding;

	// TODO реализовать свёртку повторяющихся сообщений. Решить проблему сброса кэша на диск в этом режиме.
	int mDuplicateCounter;
	QString mLastMessage;

	DestinationFilePtr mCurrentFile;
};

//---------------------------------------------------------------------------
