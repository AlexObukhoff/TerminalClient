/* @file Реализация класса-запроса для скачивания файла с докачкой. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QUrl>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

#include "NetworkTask.h"

//------------------------------------------------------------------------
class FileDownloadTask : public NetworkTask
{
	Q_OBJECT

public:
	FileDownloadTask(const QUrl & aUrl, const QString & aPath);

	QString getPath() const;

	void closeFile();

public slots:
	/// Удаляем файл и создаем его заново
	void resetFile();

protected:
	QUrl    mUrl;
	QString mPath;
};

//------------------------------------------------------------------------
