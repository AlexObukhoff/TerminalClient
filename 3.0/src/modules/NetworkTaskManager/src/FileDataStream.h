/* @file Реализация файлового потока данных. */

#pragma once

#include "DataStream.h"

class QByteArray;
class QString;

//------------------------------------------------------------------------
class FileDataStream : public DataStream
{
public:
	FileDataStream(const QString & aPath);

	virtual bool clear();
	virtual bool write(const QByteArray & aData);

	virtual qint64 size() const;
};

//------------------------------------------------------------------------
