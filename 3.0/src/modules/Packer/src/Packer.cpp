/* @file Класс для архивации/разархивации папок и файлов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QTextCodec>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILog.h>

// Project
#include "Packer.h"

// Qt 3rdparty
#include <zlib.h>

//------------------------------------------------------------------------------
namespace CPacker
{
	const int DefaultTimeout = 300 * 1000;  // таймаут распаковки/запаковки в мс.
	
	// ported from https://stackoverflow.com/questions/2690328/qt-quncompress-gzip-data
	const int GZIP_WINDOWS_BIT = 15 + 16;
	const int GZIP_CHUNK_SIZE = 32 * 1024;
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
bool Packer::gzipCompress(const QByteArray & aInBuffer, const QString & aFileName, QByteArray & aOutBuffer, int aLevel)
{
	// Prepare output
	aOutBuffer.clear();

	// Is there something to do?
	if (aInBuffer.length())
	{
		// Declare vars
		int flush = 0;

		// Prepare deflater status
		z_stream strm;
		memset(&strm, 0, sizeof(strm));

		// Initialize deflater
		int ret = deflateInit2(&strm, qMax(-1, qMin(9, aLevel)), Z_DEFLATED, CPacker::GZIP_WINDOWS_BIT, 8, Z_DEFAULT_STRATEGY);

		if (ret != Z_OK)
		{
			return false;
		}

		gz_header header;
		memset(&header, 0, sizeof(header));
		QByteArray nameBuffer = aFileName.toLatin1();
		nameBuffer.append('\0');
		header.name = (Bytef *)nameBuffer.constData();
		header.name_max = nameBuffer.size();
		header.time = QDateTime::currentDateTime().toTime_t();

		ret = deflateSetHeader(&strm, &header);
		if (ret != Z_OK)
		{
			return false;
		}

		// Prepare output
		aOutBuffer.clear();

		// Extract pointer to input data
		const char * input_data = aInBuffer.data();
		int input_data_left = aInBuffer.length();

		// Compress data until available
		do
		{
			// Determine current chunk size
			int chunk_size = qMin(CPacker::GZIP_CHUNK_SIZE, input_data_left);

			// Set deflater references
			strm.next_in = (unsigned char*)input_data;
			strm.avail_in = chunk_size;

			// Update interval variables
			input_data += chunk_size;
			input_data_left -= chunk_size;

			// Determine if it is the last chunk
			flush = (input_data_left <= 0 ? Z_FINISH : Z_NO_FLUSH);

			// Deflate chunk and cumulate output
			do
			{
				// Declare vars
				char out[CPacker::GZIP_CHUNK_SIZE];

				// Set deflater references
				strm.next_out = (unsigned char*)out;
				strm.avail_out = CPacker::GZIP_CHUNK_SIZE;

				// Try to deflate chunk
				ret = deflate(&strm, flush);

				// Check errors
				if (ret == Z_STREAM_ERROR)
				{
					// Clean-up
					deflateEnd(&strm);

					// Return
					return false;
				}

				// Determine compressed size
				int have = (CPacker::GZIP_CHUNK_SIZE - strm.avail_out);

				// Cumulate result
				if (have > 0)
				{
					aOutBuffer.append((char*)out, have);
				}
			} while (strm.avail_out == 0);

		} while (flush != Z_FINISH);

		// Clean-up
		deflateEnd(&strm);

		// Return
		return (ret == Z_STREAM_END);
	}

	return true;
}

//------------------------------------------------------------------------------
bool Packer::gzipUncompress(const QByteArray & aInBuffer, QString & aFileName, QByteArray & aOutBuffer)
{
	// Prepare output
	aFileName.clear();
	aOutBuffer.clear();

	// Is there something to do?
	if (aInBuffer.length() > 0)
	{
		// Prepare inflater status
		z_stream strm;
		memset(&strm, 0, sizeof(strm));

		// Initialize inflater
		int ret = inflateInit2(&strm, CPacker::GZIP_WINDOWS_BIT);

		if (ret != Z_OK)
		{
			return false;
		}

		gz_header header;
		memset(&header, 0, sizeof(header));
		QByteArray nameBuffer;
		nameBuffer.fill('\0', 256);
		header.name = (Bytef *)nameBuffer.data();
		header.name_max = nameBuffer.size();

		ret = inflateGetHeader(&strm, &header);
		if (ret != Z_OK)
		{
			return false;
		}

		// Extract pointer to aInBuffer data
		const char * input_data = aInBuffer.data();
		int input_data_left = aInBuffer.length();

		// Decompress data until available
		do
		{
			// Determine current chunk size
			int chunk_size = qMin(CPacker::GZIP_CHUNK_SIZE, input_data_left);

			// Check for termination
			if (chunk_size <= 0)
			{
				break;
			}

			// Set inflater references
			strm.next_in = (unsigned char*)input_data;
			strm.avail_in = chunk_size;

			// Update interval variables
			input_data += chunk_size;
			input_data_left -= chunk_size;

			// Inflate chunk and cumulate output
			do
			{
				// Declare vars
				char out[CPacker::GZIP_CHUNK_SIZE];

				// Set inflater references
				strm.next_out = (unsigned char*)out;
				strm.avail_out = CPacker::GZIP_CHUNK_SIZE;

				// Try to inflate chunk
				ret = inflate(&strm, Z_NO_FLUSH);

				switch (ret)
				{
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
				case Z_STREAM_ERROR:
					inflateEnd(&strm);  // Clean-up
					return false;
				}

				// Determine decompressed size
				int have = (CPacker::GZIP_CHUNK_SIZE - strm.avail_out);

				// Cumulate result
				if (have > 0)
				{
					aOutBuffer.append((char*)out, have);
				}

			} while (strm.avail_out == 0);

		} while (ret != Z_STREAM_END);

		if (strlen((const char *)header.name))
		{
			aFileName = QString::fromLatin1((const char *)header.name);
		}

		// Clean-up
		inflateEnd(&strm);

		// Return
		return (ret == Z_STREAM_END);
	}

	return true;
}

//------------------------------------------------------------------------------
QString Packer::pack(const QString & aTargetName, const QString & aSourceDir, const QStringList & aSearchMasks, const QStringList & aExcludeWildcard)
{
	QStringList files = pack(aTargetName, aSourceDir, aSearchMasks, aExcludeWildcard, 0);

	return files.isEmpty() ? QString() : files.first();
}

//------------------------------------------------------------------------------
QStringList Packer::pack(const QString & aTargetName, const QString & aSourceDir, const QStringList & aSearchMasks, const QStringList & aExcludeWildcard, int aMaxPartSize)
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
bool Packer::unpack(const QString & aSourceName, const QString & aDestinationDir, bool aSkipExisting, const QStringList & aExtractFiles /*= QStringList()*/)
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
