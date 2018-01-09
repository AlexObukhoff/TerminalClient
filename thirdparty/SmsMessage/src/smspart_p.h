/* @file Часть смс сообщения. */

#pragma once

// Qt
#include <QtCore/QString>
#include <QtCore/QDateTime>

class SmsPart;

//------------------------------------------------------------------------------
class SmsPartData : public QSharedData
{
public:
	enum CodingScheme {
		SevenBitScheme = 0x0,
		EightBitScheme = 0x4,
		Usc2Scheme = 0x8
	};

	SmsPartData();
	SmsPartData(const SmsPartData & aOther);
	virtual ~SmsPartData();

	QByteArray mPartId;
	int mPartNumber;
	int mPartCount;
	QString mSenderAddress;
	QDateTime mDateTime;
	SmsPart::EMessageType mMessageType;
	CodingScheme mCodingScheme;
	QString mText;
	bool mHasErrors;
	QString mErrorString;
	bool mHasUserDataHeader;
	unsigned char mUserDataHeaderLength;
};
//------------------------------------------------------------------------------

QDebug operator <<(QDebug aDebug, const SmsPart & aSmsPart);
QDebug operator<<(QDebug aDebug, SmsPart * aSmsPart);

