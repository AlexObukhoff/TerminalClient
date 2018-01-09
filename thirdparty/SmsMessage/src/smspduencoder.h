/* @file Декодер сообщения в формат PDU. */

#pragma once

// Qt
#include <QtCore/QString>

//------------------------------------------------------------------------------
class SmsPduEncoder
{
public:
	/// Packs an unpacked 7 bit array to an 8 bit packed array according to the GSM
	static QByteArray encode(const QByteArray & aUnpackedBytes, quint8 aDefaultByte = ' ');

protected:
	static quint8 mEncodeMask[];
};
//------------------------------------------------------------------------------
