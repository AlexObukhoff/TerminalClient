/* @file Функционал Tag-Len-Value. 

         Based on code from https://github.com/lumag/emv-tools/ (c) lumag
*/

// Project
#include "TLV.h"

//------------------------------------------------------------------------------
bool EMV::TLV::TLVs::parse(const QByteArray & aBuffer)
{
	QByteArray copy = aBuffer;

	return !parseItem(copy).isEmpty();
}

//------------------------------------------------------------------------------
quint16 EMV::TLV::TLVs::takeByte(QByteArray & aBuffer)
{
	quint16 result = 0;

	if (aBuffer.size() > 0 )
	{
		result = *((quint8 *)aBuffer.data());
		aBuffer.remove(0, 1);
	}

	return result;
}

//------------------------------------------------------------------------------
EMV::TLV::SItem EMV::TLV::TLVs::parseItem(QByteArray & aBuffer)
{
	quint16 tag = parseTag(aBuffer);
	quint16 len = parseLen(aBuffer);

	if (len > aBuffer.size() || len == Len::Invalid || tag == Tag::Invalid)
	{
		return SItem();
	}

	SItem it;
	it.tag = tag;
	it.body = aBuffer.left(len);
	aBuffer.remove(0, len);

	mItems << it;

	if (it.isComplex())
	{
		QByteArray copy = it.body;

		while (copy.size() > 0) 
		{
			SItem it2 = parseItem(copy);

			if (it2.isEmpty())
			{
				break;
			}
		}
	}

	return it;
}

//------------------------------------------------------------------------------
EMV::TLV::SItem EMV::TLV::TLVs::getTag(quint16 aTag)
{
	foreach (auto i, mItems)
	{
		if (i.tag == aTag)
		{
			return i;
		}
	}

	return SItem();
}

//------------------------------------------------------------------------------
quint16 EMV::TLV::TLVs::parseLen(QByteArray & aBuffer)
{
	if (aBuffer.isEmpty())
	{
		return Len::Invalid;
	}

	quint16 l = takeByte(aBuffer);

	if (!(l & Len::Long))
	{
		return l;
	}

	quint16 ll = l &~ Len::Long;

	if (aBuffer.size() < ll)
	{
		return Len::Invalid;
	}

	/* FIXME - WTF? */
	if (ll != 1)
	{
		return Len::Invalid;
	}

	return takeByte(aBuffer);
}

//------------------------------------------------------------------------------
quint16 EMV::TLV::TLVs::parseTag(QByteArray & aBuffer)
{
	if (aBuffer.isEmpty())
	{
		return Tag::Invalid;
	}

	quint16 tag = takeByte(aBuffer);

	if ((tag & Tag::ValueMask) != Tag::ValueCont)
	{
		return tag;
	}

	if (aBuffer.isEmpty())
	{
		return Tag::Invalid;
	}

	tag |= takeByte(aBuffer) << 8;

	return tag;
}

//------------------------------------------------------------------------------
EMV::TLV::TLVs::TLVs(const QByteArray & aBuffer)
{
	parse(aBuffer);
}

//------------------------------------------------------------------------------
