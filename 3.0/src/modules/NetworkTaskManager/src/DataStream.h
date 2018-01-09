/* @file Реализация базового класса потока данных. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QIODevice>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

class QByteArray;

//------------------------------------------------------------------------
class DataStream
{
public:
	DataStream(QIODevice * aDevice);
	virtual ~DataStream();

	virtual bool clear();
	virtual bool seek(qint64 aOffset);
	virtual bool write(const QByteArray & aData);

	virtual QByteArray takeAll();
	virtual QByteArray readAll();

	virtual qint64 size() const;

	virtual void close();

protected:
	QSharedPointer<QIODevice> m_stream;
};

//------------------------------------------------------------------------
