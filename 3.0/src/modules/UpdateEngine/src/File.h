/* @file Класс файл в компоненте дистрибутива. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QDir>
#include <Common/QtHeadersEnd.h>

/// Структура описывает параметры файла обновления
class File
{
public:
	typedef enum 
	{
		OK,                  // уже скачан, качать не нужно
		NotFullyDownloaded,  // нужно докачать
		Error                // нужно удалить и качать заново
	} Result;

public:
	File();
	File(const QString & aName, const QString & aHash, const QString & aUrl, qint64 aSize = 0);

	bool operator==(const File & aFile) const;

	/// Проверить, нужно ли скачивать файл
	Result verify(const QString & aTempFilePath);

public:
	const QString & name() const { return mName; }
	const QString & url()  const { return mUrl;  }
	const QString & hash() const { return mHash; }
	qint64          size() const { return mSize; }

	// Возвращает корневую папку файла, или само имя файла
	QString dir() const { return QDir::fromNativeSeparators(mName).split("/").at(0); }

private:
	QString mName;  // путь относительно корня дистрибутива
	QString mHash;  // sha256 хеш файла
	qint64  mSize;  // Размер файла 
	QString mUrl;   // Опциональный параметр - адрес для скачивания
};

inline uint qHash(const File & aFile)
{ 
    uint h1 = qHash(aFile.name());
    uint h2 = qHash(aFile.hash());
    return ((h1 << 16) | (h1 >> 16)) ^ h2;
}

// Список файлов.
typedef QSet<File> TFileList;

