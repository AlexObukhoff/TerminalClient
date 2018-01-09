/* @file Декодер смс сообщения из формата PDU. */

#pragma once

// Qt
#include <QtCore/QString>

// Проект
#include "smspart_p.h"
#include "sms_p.h"
#include "sms.h"

//------------------------------------------------------------------------------
class SmsPduDecoderPrivate
{
public:
	enum EAddressType
	{
		UnknownNumber = 0x0,
		InternationalNumber = 0x10,
		NationalNumber = 0x20,
		NetworkNumber = 0x30,
		SubscriberNumber = 0x40,
		Alphanumeric = 0x50,
		AbbreviatedNumber = 0x60
	};

	enum EMessageType
	{
		MessageDeliver = 0x0,
		StatusReport = 0x2,
		SubmitReport = 0x1
	};

	SmsPduDecoderPrivate();
	virtual ~SmsPduDecoderPrivate();
	SmsPart decode(const QByteArray & aData);
	bool readServiceNumber();
	bool readMessageType();
	bool readSenderAddress();
	bool readProtocolIdentifier();
	bool readDataCodingScheme();
	bool readTimestamp();
	bool readUserDataHeader();
	bool readUserData();
	QString decodeInternationalNumber(const QByteArray & aNumber) const;
	void setAddressType(char aType);
	bool read(unsigned char *aBuf);
	QByteArray read(int aSize);
	void skipOctet();
	int septetsSize(int aOctetsSize);
	bool reportError(const QString & aMessage);

	int mCount;
	QByteArray mData;
    QScopedPointer<SmsPartData> mSmsPartData;
};
//------------------------------------------------------------------------------
