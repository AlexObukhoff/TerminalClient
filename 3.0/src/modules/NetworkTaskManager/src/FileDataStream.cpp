/* @file Реализация файлового потока данных. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <Common/QtHeadersEnd.h>

// Project
#include "FileDataStream.h"

//------------------------------------------------------------------------
FileDataStream::FileDataStream(const QString & aPath)
	: DataStream(nullptr)
{
	m_stream = QSharedPointer<QIODevice>(new QFile(aPath));
	m_stream->open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Unbuffered);
}

//------------------------------------------------------------------------
bool FileDataStream::clear()
{
	QFile * file = dynamic_cast<QFile *>(m_stream.data());

	return file->resize(0) && file->seek(0);
}

//------------------------------------------------------------------------
bool FileDataStream::write(const QByteArray & aData)
{
	QFile * file = dynamic_cast<QFile *>(m_stream.data());

	return (file->write(aData) == aData.size() && file->flush());
}

//------------------------------------------------------------------------
qint64 FileDataStream::size() const
{
	QFile * file = dynamic_cast<QFile *>(m_stream.data());

	return file->size();
}

//------------------------------------------------------------------------
