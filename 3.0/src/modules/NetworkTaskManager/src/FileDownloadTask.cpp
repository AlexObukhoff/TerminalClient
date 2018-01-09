/* @file Реализация класса-запроса для скачивания файла с докачкой. */

//Qt 
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <Common/QtHeadersEnd.h>

// Project
#include "FileDataStream.h"
#include "FileDownloadTask.h"

//------------------------------------------------------------------------
FileDownloadTask::FileDownloadTask(const QUrl & aUrl, const QString & aPath)
	: mUrl(aUrl),
	  mPath(aPath)
{
	setUrl(mUrl);
	setDataStream(new FileDataStream(mPath));
	setFlags(NetworkTask::Continue);
}

//------------------------------------------------------------------------
QString FileDownloadTask::getPath() const
{
	return mPath;
}

//------------------------------------------------------------------------
void FileDownloadTask::closeFile()
{
	getDataStream()->close();
}

//------------------------------------------------------------------------
void FileDownloadTask::resetFile()
{
	closeFile();

	QFile::remove(mPath);

	setDataStream(new FileDataStream(mPath));
}

//------------------------------------------------------------------------
