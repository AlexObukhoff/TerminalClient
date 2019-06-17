/* @file Реализация простого логгера в файл. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDebug>
#include <QtCore/QThread>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")
#endif

#include <Common/Version.h>
#include <SysUtils/ISysUtils.h>

// Проект
#include "SimpleLog.h"

//---------------------------------------------------------------------------
SimpleLog::SimpleLog(const QString& aName, LogType::Enum aType, LogLevel::Enum aMaxLogLevel) :
	mInitOk(false),
	mMaxLogLevel(aMaxLogLevel),
	mName(aName),
	mDestination(aName),
	mType(aType),
	mPadding(0),
	mDuplicateCounter(0)
{
}

//---------------------------------------------------------------------------
SimpleLog::~SimpleLog()
{
	if (mInitOk)
	{
		// write log footer
		adjustPadding(-99);

		// Пишем в лог footer только если уже была инициализация
		safeWrite(LogLevel::Normal, QString("%1 LOG [%2] STOP.").arg(QString("*").repeated(32)).arg(getName()));
	}
}

//---------------------------------------------------------------------------
const QString & SimpleLog::getName() const
{
	return mName;
}

//---------------------------------------------------------------------------
const QString & SimpleLog::getDestination() const
{
	return mDestination;
}

//---------------------------------------------------------------------------
void SimpleLog::setDestination(const QString & aDestination)
{
	switch (mType)
	{
	case LogType::File:
	{
		if (mDestination != aDestination)
		{
			mDestination = aDestination;
			mInitOk = false;
		}

		break;
	}

	case LogType::Console:
	case LogType::Debug:
		break;

	default:
		break;
	}
}

//---------------------------------------------------------------------------
LogType::Enum SimpleLog::getType() const
{
	return mType;
}

//---------------------------------------------------------------------------
void SimpleLog::setLevel(LogLevel::Enum aLevel)
{
	mMaxLogLevel = aLevel;
}

//---------------------------------------------------------------------------
void SimpleLog::adjustPadding(int aStep)
{
	mPadding = qMax(0, mPadding + aStep);
}

//---------------------------------------------------------------------------
void SimpleLog::logRotate()
{
	if (!isInitiated())
	{
		init();
	}
}

//---------------------------------------------------------------------------
void SimpleLog::write(LogLevel::Enum aLevel, const QString & aMessage)
{
	if (!isInitiated() && !init())
	{
		return;
	}

	if (aLevel > mMaxLogLevel)
	{
		return;
	}

	safeWrite(aLevel, aMessage);
}

//---------------------------------------------------------------------------
void SimpleLog::write(LogLevel::Enum aLevel, const QString & aMessage, const QByteArray & aData)
{
	write(aLevel, aMessage + aData.toHex().data());
}

//---------------------------------------------------------------------------
bool SimpleLog::init()
{
	static QMutex fileListMutex(QMutex::Recursive);
	static QMap<QString, DestinationFilePtr> fileList;

	bool needWriteHeader = false;

	switch (mType)
	{
	case LogType::File:
	{
		QMutexLocker locker(&fileListMutex);

		if (fileList.contains(mDestination))
		{
			mCurrentFile = fileList[mDestination];

			if (!mCurrentFile || !mCurrentFile->isOpen())
			{
				fileList.remove(mDestination);

				return init();
			}

			// Тут происходит смена даты файла при смене суток.
			if (mCurrentFile->fileName().contains(QDate::currentDate().toString("yyyy.MM.dd")))
			{
				break;
			}
			else
			{
				fileList.remove(mDestination);
				return init();
			}
		}
		else
		{
			QString logPath;
			QString workingDirectory;

#ifdef Q_OS_WIN
			TCHAR szPath[MAX_PATH] = { 0 };
			
			if (GetModuleFileName(0, szPath, MAX_PATH))
			{
				QFileInfo info(QDir::toNativeSeparators(QString::fromWCharArray(szPath)));
				QString settingsFilePath = QDir::toNativeSeparators(info.absolutePath() + "/" + info.completeBaseName() + ".ini");
				QSettings mSettings(ISysUtils::rmBOM(settingsFilePath), QSettings::IniFormat);
				mSettings.setIniCodec("UTF-8");

				if (mSettings.contains("common/working_directory"))
				{
					QString directory = mSettings.value("common/working_directory").toString();
					workingDirectory = QDir::toNativeSeparators(QDir::cleanPath((QDir::isAbsolutePath(directory) ? "" : (info.absolutePath() + "/")) + directory));
				}
				else
				{
					workingDirectory = info.absolutePath();
				}
			}
#else
#pragma error
#endif
			logPath = workingDirectory + "/logs/" + QDate::currentDate().toString("yyyy.MM.dd ") + mDestination + ".log";

			QFileInfo logPathInfo(logPath);

			if (!logPathInfo.exists())
			{
				QDir logDir(logPathInfo.absolutePath());

				if (!logDir.mkpath(logPathInfo.absolutePath()))
				{
					return false;
				}
			}

			mCurrentFile = DestinationFilePtr(new DestinationFile());

			if (!mCurrentFile->open(logPath))
			{
				return false;
			}

			needWriteHeader = true;

			fileList[mDestination] = mCurrentFile;
			break;
		}
	}

	case LogType::Console:
	case LogType::Debug:
		break;

	default:
		return false;
	}

	mInitOk = true;

	if (needWriteHeader)
	{
		writeHeader();
	}

	return true;
}

//---------------------------------------------------------------------------
bool SimpleLog::isInitiated()
{
	// Тут проверяем сменился ли день
	if (mType == LogType::File && mCurrentFile && !mCurrentFile->fileName().contains(QDate::currentDate().toString("yyyy.MM.dd")))
	{
		return false;
	}

	return mInitOk;
}

//---------------------------------------------------------------------------
void SimpleLog::writeHeader()
{
	write(LogLevel::Normal, QString("%1 LOG [%2] STARTED. %3 %4.")
		.arg(QString("*").repeated(32)).arg(getName())
		.arg(Cyberplat::Application).arg(Cyberplat::getVersion()));
}

//---------------------------------------------------------------------------
void SimpleLog::safeWrite(LogLevel::Enum aLevel, const QString & aMessage)
{
	if (!mInitOk)
	{
		return;
	}

	QString formattedMessage = QTime::currentTime().toString("hh:mm:ss.zzz ");

	formattedMessage += QString("T:%1 ").arg((quint32)QThread::currentThreadId(), 8, 16, QLatin1Char('0'));

	switch (aLevel)
	{
	case LogLevel::Normal:
		formattedMessage += "[I]";
		break;

	case LogLevel::Warning:
		formattedMessage += "[W]";
		break;

	case LogLevel::Error:
		formattedMessage += "[E]";
		break;

	case LogLevel::Fatal:
		formattedMessage += "[F]";
		break;

	case LogLevel::Debug:
		formattedMessage += "[D]";
		break;

	case LogLevel::Trace:
		formattedMessage += "[T]";
		break;

	default:
		formattedMessage += "[U]";
		break;
	}

	formattedMessage += QString(" %1%2\n").arg("", mPadding * 3).arg(aMessage);

	switch (mType)
	{
	case LogType::File:
		mCurrentFile->write(formattedMessage);
		break;

	case LogType::Debug:
		qDebug() << formattedMessage;
		break;

	case LogType::Console:
		printf("%s", formattedMessage.toLocal8Bit().data());
		break;

	default:
		break;
	}
}

//---------------------------------------------------------------------------
