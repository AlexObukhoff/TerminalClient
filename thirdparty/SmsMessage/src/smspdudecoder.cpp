/* @file Декодер смс сообщения из формата PDU. */

// Qt
#include <QtCore/QDebug>
#include <QtCore/QTextCodec>

// Проект
#include "smspdudecoder.h"
#include "smspdudecoder_p.h"

//------------------------------------------------------------------------------
SmsPduDecoder::SmsPduDecoder() :
	d_ptr(new SmsPduDecoderPrivate)
{
}

//------------------------------------------------------------------------------
SmsPduDecoder::~SmsPduDecoder()
{
}

//------------------------------------------------------------------------------
SmsPart SmsPduDecoder::decodeMessage(const QByteArray & aData)
{
	Q_D(SmsPduDecoder);

	return d->decode(aData);
}

//------------------------------------------------------------------------------
QByteArray SmsPduDecoder::septetsToOctets(const QByteArray & aSeptets, int aSize)
{
	QByteArray octets;
	unsigned char delta = 0;
	for (int i = 0, shift = 1; i < aSeptets.size(); i++, shift++)
	{	
		unsigned char currentSeptet = aSeptets[i];

		unsigned char mask = (0xff >> shift);
		unsigned char temp = currentSeptet & mask;

		unsigned char currentDelta = currentSeptet >> (8 - shift);

		if (shift == 1)
		{
			octets.append(temp);
			delta = currentDelta;
			continue;
		}

		temp = temp << (shift - 1);
		unsigned char octet = temp | delta;
		delta = currentDelta;
		octets.append(octet);

		if (shift == 7)
		{
			shift = 0;
			octets.append(delta);
		}
	}

	int octetsSizeDelta = octets.size() - aSize;
	if (octetsSizeDelta > 0)
	{
		octets.chop(octetsSizeDelta);
	}
	return octets;
}

//SmsPduDevoderPrivate
//------------------------------------------------------------------------------
SmsPduDecoderPrivate::SmsPduDecoderPrivate()
{
}

//------------------------------------------------------------------------------
SmsPduDecoderPrivate::~SmsPduDecoderPrivate()
{
}

//------------------------------------------------------------------------------
SmsPart SmsPduDecoderPrivate::decode(const QByteArray & aData)
{
	mCount = 0;
	mData = aData;
    mSmsPartData.reset(new SmsPartData);

	if (readServiceNumber()
			&& readMessageType()
			&& readSenderAddress()
			&& readProtocolIdentifier()
			&& readDataCodingScheme()
			&& readTimestamp()
			&& readUserData())
	{
        return SmsPart(mSmsPartData.take());
	} 
	else 
	{
        return SmsPart(mSmsPartData.take());
	}
}

//------------------------------------------------------------------------------
bool SmsPduDecoderPrivate::readServiceNumber()
{
	//skip service number
	unsigned char serviceNumberLength;

	if (!read(&serviceNumberLength))
	{
		return reportError(QObject::tr("Error reading service number."));
	}

	return !read(serviceNumberLength).isEmpty();
}

//------------------------------------------------------------------------------
bool SmsPduDecoderPrivate::readMessageType()
{
	unsigned char messageType;

	if (!read(&messageType))
	{
		return reportError(QObject::tr("Error reading message type."));
	}

    mSmsPartData->mMessageType = static_cast<SmsPart::EMessageType>(messageType & 0x2); // xxxxxx00
    mSmsPartData->mHasUserDataHeader = (messageType & 0x40) != 0; //x1xxxxxx

	return true;
}

//------------------------------------------------------------------------------
bool SmsPduDecoderPrivate::readSenderAddress()
{
	unsigned char addressLength;
	unsigned char addressByte;
	if (!read(&addressLength))
	{
		return reportError(QObject::tr("Error reading address length."));
	}

	if (!read(&addressByte))
	{
		return reportError(QObject::tr("Error reading address type."));
	}

	unsigned char addressTypeByte = addressByte & 0x70; // x111xxxx

	if (addressTypeByte == InternationalNumber)
	{
		if (addressLength % 2)
		{
			addressLength++;
		}
		addressLength /= 2;
		QByteArray rawSenderNumber = read(addressLength);
		if (rawSenderNumber.isEmpty())
		{
			return reportError(QObject::tr("Error reading sender address."));
		}
        mSmsPartData->mSenderAddress = decodeInternationalNumber(rawSenderNumber);
	}
	else if (addressTypeByte == Alphanumeric)
	{
		if (addressLength % 2)
		{
			addressLength++;
		}
		addressLength /= 2;
		QByteArray address = read(addressLength);
		if (address.isEmpty())
		{
			return reportError(QObject::tr("Error reading sender address."));
		}
        mSmsPartData->mSenderAddress = SmsPduDecoder::septetsToOctets(address, addressLength);
	}
	else if (addressTypeByte == NetworkNumber || addressTypeByte == SubscriberNumber)
	{
		if (addressLength % 2)
		{
			addressLength++;
		}
		addressLength /= 2;
		QByteArray rawSenderNumber = read(addressLength);
		if (rawSenderNumber.isEmpty())
		{
			return reportError(QObject::tr("Error reading sender address."));
		}
        mSmsPartData->mSenderAddress = decodeInternationalNumber(rawSenderNumber);

	}
	else
	{
		return reportError(QObject::tr("Error reading address type %1.").arg(addressTypeByte));
	}

	return true;
}

//------------------------------------------------------------------------------
bool SmsPduDecoderPrivate::readProtocolIdentifier()
{
	unsigned char id;

	return read(&id);
}

//------------------------------------------------------------------------------
bool SmsPduDecoderPrivate::readDataCodingScheme()
{
	unsigned char codingScheme;

	if (!read(&codingScheme))
	{
		return reportError(QObject::tr("Error reading data coding scheme."));
	}

    mSmsPartData->mCodingScheme = static_cast<SmsPartData::CodingScheme>(codingScheme & 0xc); // xxxxx11xx

	return true;
}

//------------------------------------------------------------------------------
bool SmsPduDecoderPrivate::readTimestamp()
{
	QByteArray timestamp = read(6); //exclude 1 byte of timezone
	if (timestamp.isEmpty())
	{
		return reportError(QObject::tr("Error reading timestamp."));
	}

	skipOctet();
	QString timestampString = SmsPduDecoder::swapSemiOctets(timestamp).toHex();

	timestampString.prepend("20");
    mSmsPartData->mDateTime = QDateTime::fromString(timestampString, "yyyyMMddHHmmss");

	return true;
}

//------------------------------------------------------------------------------
bool SmsPduDecoderPrivate::readUserDataHeader()
{
    if (!mSmsPartData->mHasUserDataHeader)
	{
        mSmsPartData->mPartCount = 1;
        mSmsPartData->mPartId = QByteArray();
        mSmsPartData->mPartNumber = 0;
		return false;
	}

	unsigned char headerLength;
	read(&headerLength);
    mSmsPartData->mUserDataHeaderLength = headerLength;

	unsigned char iei;
	read(&iei);

	unsigned char iedl;
	read(&iedl);

	char numberLength;
	if (iei == 0x0)
	{
		numberLength = 1;
	}
	else if (iei == 0x8)
	{
		numberLength = 2;
	}
	else
	{
		return reportError("Error reading user header data.");
	}

	QByteArray partId = read(numberLength);
	unsigned char partCount;
	read(&partCount);
	unsigned char partNumber;
	read(&partNumber);

    mSmsPartData->mPartCount = partCount;
    mSmsPartData->mPartId = partId;
    mSmsPartData->mPartNumber = partNumber - 1;

	return true;
}

//------------------------------------------------------------------------------
bool SmsPduDecoderPrivate::readUserData()
{
	unsigned char userDataLength;
	if (!read(&userDataLength))
	{
		return reportError(QObject::tr("Error reading user data length."));
	}

	if (readUserDataHeader())
	{
        userDataLength = userDataLength - mSmsPartData->mUserDataHeaderLength - 1;
	}

    if (mSmsPartData->mCodingScheme == SmsPartData::SevenBitScheme)
	{
		QByteArray message = read(septetsSize(userDataLength));
		if (message.isEmpty())
		{
			return reportError(QObject::tr("Error reading text of message."));
		}
		QByteArray buf = SmsPduDecoder::septetsToOctets(message, userDataLength);
        mSmsPartData->mText = buf;
	}
    else if (mSmsPartData->mCodingScheme == SmsPartData::EightBitScheme)
	{
        mSmsPartData->mText = read(userDataLength);
	}
    else if (mSmsPartData->mCodingScheme == SmsPartData::Usc2Scheme)
	{
		QByteArray message = read(userDataLength);
		QTextCodec * codec = QTextCodec::codecForName("UTF-16BE");
        mSmsPartData->mText = codec->toUnicode(message);
	}
	else
	{
        mSmsPartData->mText = read(userDataLength);
	}
	return true;
}

//------------------------------------------------------------------------------
QString SmsPduDecoderPrivate::decodeInternationalNumber(const QByteArray & aNumber) const
{
	QByteArray buf = SmsPduDecoder::swapSemiOctets(aNumber);

	QString result = buf.toHex();
	if (result.endsWith("f", Qt::CaseInsensitive))
	{
		result.chop(1);
	}

	result.prepend("+");
	return result;
}

//------------------------------------------------------------------------------
bool SmsPduDecoderPrivate::read(unsigned char * aBuf)
{
	if (!aBuf)
	{
		return false;
	}

	if (mCount < (mData.size() - 1))
	{
		*aBuf = mData.at(mCount++);
		return true;
	}
    mSmsPartData->mHasErrors = true;
	return false;
}

//------------------------------------------------------------------------------
QByteArray SmsPduDecoderPrivate::read(int aSize)
{
	if ((aSize > 0) && ((mCount + aSize - 1) < mData.size()))
	{
		int i = mCount;
		mCount += aSize;
		return mData.mid(i, aSize);
	}

    mSmsPartData->mHasErrors = true;
	return QByteArray();
}

//------------------------------------------------------------------------------
void SmsPduDecoderPrivate::skipOctet()
{
	if (mCount < mData.size() - 1)
	{
		mCount++;
	}
}

//------------------------------------------------------------------------------
int SmsPduDecoderPrivate::septetsSize(int aOctetsSize)
{
	return static_cast<int>(ceil(aOctetsSize * 0.875));
}

//------------------------------------------------------------------------------
bool SmsPduDecoderPrivate::reportError(const QString & aMessage)
{
    mSmsPartData->mHasErrors = true;
    mSmsPartData->mErrorString = QString("SmsPduDecoder: %1").arg(aMessage);
	return false;
}
//------------------------------------------------------------------------------
