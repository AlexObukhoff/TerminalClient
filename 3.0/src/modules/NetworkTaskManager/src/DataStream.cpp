/* @file Реализация базового класса потока данных. */

// Project
#include "DataStream.h"

//------------------------------------------------------------------------
DataStream::DataStream(QIODevice * aDevice) :
	m_stream(aDevice)
{
	if (m_stream)
	{
		m_stream->open(QIODevice::ReadWrite | QIODevice::Append);
	}
}

//------------------------------------------------------------------------
DataStream::~DataStream()
{
}

//------------------------------------------------------------------------
bool DataStream::clear()
{
	m_stream->seek(0);

	return false;
}

//------------------------------------------------------------------------
bool DataStream::seek(qint64 aOffset)
{
	return m_stream->seek(aOffset);
}

//------------------------------------------------------------------------
bool DataStream::write(const QByteArray & aData)
{
	return m_stream->write(aData) == aData.size();
}

//------------------------------------------------------------------------
QByteArray DataStream::takeAll()
{
	m_stream->seek(0);
	
	QByteArray result = m_stream->readAll();

	clear();

	return result;
}

//------------------------------------------------------------------------
QByteArray DataStream::readAll()
{
	m_stream->seek(0);
	
	return m_stream->readAll();
}

//------------------------------------------------------------------------
qint64 DataStream::size() const
{
	return m_stream->size();
}

//------------------------------------------------------------------------
void DataStream::close()
{
	m_stream->close();
}

//------------------------------------------------------------------------
