/* @file Декодер смс сообщения из формата PDU. */

#pragma once

// Qt
#include <QtCore/QString>
#include <QtCore/QDateTime>

// Проект
#include "sms.h"
#include "smspart.h"

class SmsPduDecoderPrivate;

//------------------------------------------------------------------------------
class SmsPduDecoder
{
public:
	SmsPduDecoder();
	virtual ~SmsPduDecoder();

	SmsPart decodeMessage(const QByteArray & aData);
	QList<Sms> decodeListMessages(const QByteArray & aData);
	static QByteArray septetsToOctets(const QByteArray & aSeptets, int aSize);

	inline static QByteArray swapSemiOctets(QByteArray aSemiOctets)
	{
		int size = aSemiOctets.size();
		unsigned char * data = reinterpret_cast<unsigned char *>(aSemiOctets.data());

		for (int i = 0; i < size; i++)
		{
			data[i] = ((data[i] >> 4) | (data[i] << 4)); // 0x38 -> 0x83
		}

		return QByteArray(reinterpret_cast<char *>(data), size);
	}

private:
	const QScopedPointer<SmsPduDecoderPrivate> d_ptr;
	Q_DECLARE_PRIVATE(SmsPduDecoder)
};
//------------------------------------------------------------------------------

