/* @file Реализация задачи архивации журнальных файлов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFileInfo>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>

// Модули
#include <System/IApplication.h>
#include <Common/Application.h>
#include <Packer/Packer.h>

// Проект
#include "LogArchiver.h"

namespace PPSDK = SDK::PaymentProcessor;

namespace CLogArchiver
{
	const QString DateFormat = "yyyy.MM.dd";
	const int BytesInMB = 1048576; // 2^20
}

//---------------------------------------------------------------------------
bool isArchive(const QFileInfo & aFileInfo)
{
	return aFileInfo.suffix().toLower() == "zip" || 
		aFileInfo.suffix().toLower() == "7z";
}

//---------------------------------------------------------------------------
LogArchiver::LogArchiver(const QString & aName, const QString & aLogName, const QString & aParams) 
	: ITask(aName, aLogName, aParams),
	ILogable(aLogName),
	mCanceled(false),
	mPacker("", nullptr)
{
	IApplication * app = dynamic_cast<IApplication *>(BasicApplication::getInstance());

	PPSDK::ICore * core = app->getCore();
	PPSDK::TerminalSettings * terminalSettings = static_cast<PPSDK::TerminalSettings *>(core->getSettingsService()->
		getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

	mMaxSize = terminalSettings->getLogsMaxSize();
	mLogDir = QDir(app->getWorkingDirectory() + "/logs");
	mKernelPath = app->getWorkingDirectory();

	mPacker.setLog(getLog());
	mPacker.setToolPath(mKernelPath);
}

//---------------------------------------------------------------------------
LogArchiver::~LogArchiver()
{
}

//---------------------------------------------------------------------------
inline uint qHash(QDate key) 
{ 
	return key.toJulianDay();
}

//---------------------------------------------------------------------------
void LogArchiver::execute()
{
	if (mMaxSize < 1 || !mLogDir.exists())
	{
		toLog(LogLevel::Error, "Failed execute: (max_size < 1) OR (log_dir not exist)");

		emit finished(mName, false);
		return;
	}

	ILog::logRotateAll();

	foreach(auto date, getDatesForPack())
	{
		if (!mCanceled)
		{
			packLogs(date);
		}
	}

	if (!mCanceled)
	{
		checkArchiveSize();
	}

	emit finished(mName, true);
}

//---------------------------------------------------------------------------
bool LogArchiver::cancel()
{
	mCanceled = true;
	mPacker.terminate();

	return true;
}

//---------------------------------------------------------------------------
bool LogArchiver::subscribeOnComplete(QObject * aReceiver, const char * aSlot)
{
	return connect(this, SIGNAL(finished(const QString &, bool)), aReceiver, aSlot);
}

//---------------------------------------------------------------------------
QString LogArchiver::logArchiveFileName(QDate aDate)
{
	return mLogDir.absoluteFilePath(QString("%1_logs.7z").arg(aDate.toString(CLogArchiver::DateFormat)));
}

//---------------------------------------------------------------------------
void LogArchiver::packLogs(QDate aDate)
{
	toLog(LogLevel::Normal, QString("Packs logs '%1'").arg(aDate.toString(CLogArchiver::DateFormat)));

	bool updateArchive = QFile::exists(logArchiveFileName(aDate));

	// pack files to archive
	mPacker.setUpdateMode(updateArchive);
	mPacker.setFormat(Packer::SevenZip);
	mPacker.setLevel(7);
	mPacker.setTimeout(60 * 60 * 1000); // 1 час в милисекундах
	mPacker.setRecursive(true);

	QStringList toCompress;
	toCompress 
		<< QString("logs/%1*").arg(aDate.toString("yyyy.MM.dd")) 
		<< QString("receipts/%1*").arg(aDate.toString("yyyy.MM.dd"));

	QStringList archiveWildcards;
	archiveWildcards << "*.zip" << "*.7z";

	if (!mPacker.pack(logArchiveFileName(aDate), mKernelPath, toCompress, archiveWildcards).isEmpty())
	{
		if (!mPacker.exitCode())
		{
			toLog(LogLevel::Normal, QString("Result code: %1; Output: %2").arg(mPacker.exitCode()).arg(mPacker.messages()));
		}
		
		toLog(LogLevel::Normal, "Pack OK");

		if (!mCanceled)
		{
			removeLogs(aDate);
		}
	}
	else
	{
		toLog(LogLevel::Error, QString("Pack failed: exitCode=%1 message='%2'").arg(mPacker.exitCode()).arg(mPacker.messages()));

		// если мы не обновляем архив - удаляем неудачный архив
		if (!updateArchive)
		{
			removeFile(QFileInfo(logArchiveFileName(aDate)));
		}
	}
}

//---------------------------------------------------------------------------
QList<QDate> LogArchiver::getDatesForPack() const
{
	QSet<QDate> result;

	foreach (auto file, mLogDir.entryInfoList(QDir::Files | QDir::Dirs, QDir::Name))
	{
		if (mCanceled)
		{
			break;
		}

		QDate date = QDate::fromString(file.fileName().left(10), CLogArchiver::DateFormat);
		if (date.isValid() && date != QDate::currentDate())
		{
			if ((file.isFile() && !isArchive(file)) || file.isDir())
			{
				result << date;
			}
		}
	}

	foreach (auto dir, QDir(mKernelPath + "/receipts").entryInfoList(QDir::Dirs, QDir::Name))
	{
		if (mCanceled)
		{
			break;
		}

		QDate date = QDate::fromString(dir.fileName().left(10), CLogArchiver::DateFormat);
		if (date.isValid() && date != QDate::currentDate())
		{
			result << date;
		}
	}

	auto list = result.toList();
	qSort(list);

	return list;
}

//---------------------------------------------------------------------------
void LogArchiver::removeLogs(QDate aDate)
{
	auto clearLogDir = [this](const QDir & aDir, const QDate & aDate)
	{
		foreach(auto file, aDir.entryInfoList(QStringList(aDate.toString("yyyy.MM.dd*")), QDir::Files))
		{
			if (!isArchive(file))
			{
				removeFile(file);
			}
		}
	};

	clearLogDir(mLogDir, aDate);

	// чистим подпапки
	foreach(auto dir, mLogDir.entryInfoList(QDir::Dirs | QDir::NoDot | QDir::NoDotDot))
	{
		clearLogDir(dir.filePath(), aDate);
	}

	auto clearSubdir = [&](QFileInfo & aFileInfo)
	{
		QDir dir(aFileInfo.absoluteFilePath());

		foreach (auto file, dir.entryInfoList(QDir::Files))
		{
			removeFile(file);
		}
	};

	QList<QDir> dirsToRemove;
	dirsToRemove << mLogDir << QDir(mKernelPath + "/receipts");

	foreach (auto dir, dirsToRemove)
	{
		foreach (auto file, dir.entryInfoList(QStringList(aDate.toString("yyyy.MM.dd*")), QDir::Dirs))
		{
			clearSubdir(file);

			toLog(LogLevel::Debug, QString("Remove dir %1").arg(file.absoluteFilePath()));

			mLogDir.rmdir(file.absoluteFilePath());
		}
	}
}

//---------------------------------------------------------------------------
void LogArchiver::checkArchiveSize()
{
	toLog(LogLevel::Normal, QString("Check logs size limit. Max size is %1 Mb").arg(mMaxSize));

	QStringList archiveWildcards;
	archiveWildcards << "*.zip" << "*.7z";

	QList<QFileInfo> files = mLogDir.entryInfoList(archiveWildcards, QDir::Files, QDir::Name);

	auto fileSizeSumm = [](const QList<QFileInfo> & files) -> qint64
	{
		qint64 summ = 0;
		foreach (auto file, files)
		{
			summ += file.size();
		}
		return summ;
	};

	while (!files.isEmpty() && fileSizeSumm(files) > mMaxSize * CLogArchiver::BytesInMB && !mCanceled)
	{
		removeFile(files.takeFirst());
	}

	toLog(LogLevel::Normal, QString("Logs archive size is %1 Mb").arg(fileSizeSumm(files) / double(CLogArchiver::BytesInMB), 0, 'f', 2));
}

//---------------------------------------------------------------------------
bool LogArchiver::removeFile(const QFileInfo & aFile)
{
	bool result = QFile::remove(aFile.absoluteFilePath());

	toLog(result ? LogLevel::Normal : LogLevel::Error, QString("Remove [%1] %2").arg(aFile.absoluteFilePath()).arg(result ? "OK" : "Error"));

	return result;
}

//---------------------------------------------------------------------------

