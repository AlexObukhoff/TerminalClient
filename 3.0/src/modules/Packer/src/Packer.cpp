/* @file Класс для архивации/разархивации папок и файлов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QTextCodec>
#include <QtCore/QCryptographicHash>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILog.h>

// Project
#include "Packer.h"
#include "Crc32.h"

namespace CPacker
{
	const int DefaultTimeout = 300 * 1000;  // таймаут распаковки/запаковки в мс.
};

//------------------------------------------------------------------------------
Packer::Packer(const QString & aToolPath, ILog * aLog)
	: mExitCode(0),
	  mUpdateMode(false),
	  mFormat(Zip),
	  mLevel(9),
	  mRecursive(false),
	  mTimeout(CPacker::DefaultTimeout),
	  mToolPath("7za.exe")
{
	if (aLog)
	{
		setLog(aLog);
	}

	setToolPath(aToolPath);
}

//------------------------------------------------------------------------------
void Packer::setToolPath(const QString & aToolPath)
{
	if (!aToolPath.isEmpty())
	{
		mToolPath = QDir::toNativeSeparators(QDir::cleanPath(aToolPath + QDir::separator() + "7za.exe"));

		toLog(LogLevel::Debug, QString("Zip tool path: '%1'").arg(mToolPath));
	}
}

//------------------------------------------------------------------------------
void Packer::setUpdateMode(bool aUpdateMode)
{
	mUpdateMode = aUpdateMode;
}

//---------------------------------------------------------------------------
QByteArray Packer::compressToGZ(const QByteArray & aInBuffer, const QString & aFileName, int aLevel)
{
	const unsigned char gzipheader[10] = { 0x1f,0x8b,8,0,0,0,0,0,2,0 };
	unsigned long crc = Crc32().fromByteArray(aInBuffer);
	quint32 len = aInBuffer.size();

	QByteArray out = qCompress(aInBuffer, aLevel);
	out.remove(0, 4); //qt makes first 4 bytes size of zipped data, i don't need it for gzip
	out.remove(0, 2); // Remove Zlib header
	out.chop(4); // Remove Zlib trailer 

	QByteArray gzBufer = QByteArray((const char *)gzipheader, 10);

	if (!aFileName.isEmpty())
	{
		gzBufer[3] = 8;

		gzBufer.append(aFileName.toLatin1());
		gzBufer.append("\0", 1);
	}

	gzBufer.append(out);
	gzBufer.append((const char *)&crc, 4);
	gzBufer.append((const char *)&len, 4);

	return gzBufer;
}

//------------------------------------------------------------------------------
QString Packer::compress(const QString & aTargetName, const QString & aSourceDir, const QStringList & aSearchMasks, const QStringList & aExcludeWildcard)
{
	QStringList files = compress(aTargetName, aSourceDir, aSearchMasks, aExcludeWildcard, 0);

	return files.isEmpty() ? QString() : files.first();
}

//------------------------------------------------------------------------------
QStringList Packer::compress(const QString & aTargetName, const QString & aSourceDir, const QStringList & aSearchMasks, const QStringList & aExcludeWildcard, int aMaxPartSize)
{
	QStringList zipArguments = QStringList() 
		<< (mUpdateMode ? "u" : "a")
		<< (mFormat == SevenZip ? "-t7z" : "-tzip")
		<< "-bd"
		<< QString("-mx=%1").arg(mLevel)
		<< "-ssw" << "-y" << aTargetName;

	if (mRecursive)
	{
		zipArguments << "-r";
	}

	foreach (auto mask, aSearchMasks)
	{
		zipArguments << aSourceDir + QDir::separator() + mask;
	}

	if (aMaxPartSize)
	{
		zipArguments << QString("-v%1b").arg(aMaxPartSize);
	}

	foreach (auto wildcard,  aExcludeWildcard)
	{
		if (!wildcard.isEmpty())
		{
			zipArguments << QString("-xr!%1").arg(wildcard);
		}
	}

	QFileInfo info(aTargetName);

	if (!mUpdateMode)
	{
		// remove old archive
		foreach (const QString & file, QDir(info.absolutePath(), info.fileName() + "*").entryList())
		{
			QFile::remove(QDir::toNativeSeparators(QDir::cleanPath(info.absolutePath() + QDir::separator() + file)));
		}
	}

	toLog(LogLevel::Normal, QString("Executing command: %1 %2").arg(mToolPath).arg(zipArguments.join(" ")));

	mZipProcess.start(mToolPath, zipArguments);
	if (!mZipProcess.waitForFinished(mTimeout))
	{
		mExitCode = -1;
		toLog(LogLevel::Error, QString("Unknown error while executing command or timeout expired(%1 sec): %2 %3")
			.arg(mTimeout/1000., 0, 'f', 1).arg(mToolPath).arg(zipArguments.join(" ")));

		return QStringList();
	}

	mExitCode = mZipProcess.exitCode();
	mMessages = QTextCodec::codecForLocale()->toUnicode(mZipProcess.readAllStandardOutput()).remove("\r");

	if (mExitCode == 1)
	{
		toLog(LogLevel::Warning, QString("Execute command have some warning: %1 %2. Return code: %3. Output stream: %4")
			.arg(mToolPath).arg(zipArguments.join(" ")).arg(mExitCode).arg(mMessages));
	}
	else 
	{
		if (mExitCode > 1)
		{
			toLog(LogLevel::Error, QString("Can't execute command: %1 %2. Return code: %3. Output stream: %4")
				.arg(mToolPath).arg(zipArguments.join(" ")).arg(mExitCode).arg(mMessages));

			return QStringList();
		}
	}

	return QDir(info.absolutePath(), info.fileName() + "*").entryList();
}

//------------------------------------------------------------------------------
bool Packer::test(const QString & aTargetName)
{
	QString zipCommand = QString("t \"%1\"").arg(QDir::toNativeSeparators(aTargetName));
	
	mZipProcess.setNativeArguments(zipCommand);
	mZipProcess.start(mToolPath);
	if (!mZipProcess.waitForFinished(mTimeout))
	{
		mExitCode = -1;
		toLog(LogLevel::Error, QString("Unknown error while executing command: %1 %2").arg(mToolPath).arg(zipCommand));

		return false;
	}

	mExitCode = mZipProcess.exitCode();
	mMessages = QTextCodec::codecForLocale()->toUnicode(mZipProcess.readAllStandardOutput()).remove("\r");

	return mExitCode == 0;
}

//------------------------------------------------------------------------------
bool Packer::extract(const QString & aSourceName, const QString & aDestinationDir, bool aSkipExisting, const QStringList & aExtractFiles /*= QStringList()*/)
{
	QStringList commanParams;

	commanParams << "x" << "-bd" << "-y";
	if (!aDestinationDir.isEmpty())
	{
		commanParams << "-o" + aDestinationDir;
	}
	if (aSkipExisting)
	{
		commanParams << "-aos";
	}

	commanParams << aSourceName;
	commanParams.append(aExtractFiles);

	mZipProcess.start(mToolPath, commanParams);

	if (!mZipProcess.waitForFinished(mTimeout))
	{
		mExitCode = -1;
		toLog(LogLevel::Error, QString("Unknown error while executing command: %1 %2.").arg(mToolPath).arg(commanParams.join(" ")));

		return false;
	}

	mExitCode = mZipProcess.exitCode();
	mMessages = QTextCodec::codecForLocale()->toUnicode(mZipProcess.readAllStandardOutput()).remove("\r");

	return mExitCode == 0;
}

//------------------------------------------------------------------------------
int Packer::exitCode() const
{
	return mExitCode;
}

//------------------------------------------------------------------------------
const QString & Packer::messages() const
{
	return mMessages;
}

//------------------------------------------------------------------------------
void Packer::setLevel(int aLevel)
{
	Q_ASSERT(aLevel >= 3 && aLevel <= 9);

	mLevel = aLevel;
}

//------------------------------------------------------------------------------
void Packer::setFormat(Packer::Format aFormat)
{
	mFormat = aFormat;
}

//------------------------------------------------------------------------------
void Packer::setTimeout(int aTimeout)
{
	mTimeout = aTimeout;
}

//------------------------------------------------------------------------------
void Packer::terminate()
{
	if (mZipProcess.state() != QProcess::NotRunning)
	{
		toLog(LogLevel::Error, "Terminate packer process.");

		mZipProcess.kill();
	}
}

//------------------------------------------------------------------------------
QString Packer::exitCodeDescription() const
{
	switch (mExitCode)
	{
	case -1: return "Error or timeout execution 7za.exe";
	case 0: return "OK";
	case 1: return "OK (1)";
	case 2: return "7zip: Fatal error";
	case 7: return "7zip: Command line error";
	case 8: return "7zip: Not enough memory for operation";
	case 255: return "7zip: User stopped the process";
	default: return QString("7zip: Unknown exit code: %1").arg(mExitCode);
	}
}

//------------------------------------------------------------------------------
void Packer::setRecursive(bool aRecursive)
{
	mRecursive = aRecursive;
}

//------------------------------------------------------------------------------
