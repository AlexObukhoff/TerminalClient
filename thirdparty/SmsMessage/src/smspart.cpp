/* @file Часть смс сообщения. */

// Qt
#include <QtCore/QDebug>

// Проект
#include "smspart.h"
#include "smspart_p.h"

//------------------------------------------------------------------------------
SmsPart::SmsPart() :
	d(new SmsPartData)
{
}

//------------------------------------------------------------------------------
SmsPart::SmsPart(const SmsPart & aOther) :
	d(aOther.d)
{
}

//------------------------------------------------------------------------------
SmsPart::SmsPart(SmsPartData * mSmsPartData) :
	d(mSmsPartData)
{
}

#ifdef Q_COMPILER_RVALUE_REFS
//------------------------------------------------------------------------------
inline SmsPart::SmsPart(SmsPart && aOther)
    : d(aOther.d)
{
    aOther.d = 0;
}

//------------------------------------------------------------------------------
inline SmsPart & SmsPart::operator=(SmsPart && aOther)
{
    qSwap(d, aOther.d);
    return *this;
}
#endif

//------------------------------------------------------------------------------
SmsPart::~SmsPart()
{
}

//------------------------------------------------------------------------------
QString SmsPart::getSenderAddress() const
{
	return d->mSenderAddress;
}

//------------------------------------------------------------------------------
QString SmsPart::getText() const
{
	return d->mText;
}

//------------------------------------------------------------------------------
QDateTime SmsPart::getDateTime() const
{
	return d->mDateTime;
}

//------------------------------------------------------------------------------
bool SmsPart::isValid() const
{
	return !d->mHasErrors;
}

//------------------------------------------------------------------------------
QString SmsPart::getErrorString() const
{
	return d->mErrorString;
}

//------------------------------------------------------------------------------
SmsPart::EMessageType SmsPart::getMessageType() const
{
	return d->mMessageType;
}

//------------------------------------------------------------------------------
int SmsPart::getPartNumber() const
{
	return d->mPartNumber;
}

//------------------------------------------------------------------------------
int SmsPart::getPartCount() const
{
	return d->mPartCount;
}

//------------------------------------------------------------------------------
QByteArray SmsPart::getPartId() const
{
	return d->mPartId;
}

//------------------------------------------------------------------------------
QDebug operator <<(QDebug aDebug, const SmsPart & aSmsPart)
{
	if (!aSmsPart.isValid())
	{
		aDebug.nospace() << "SmsPart("
					  << aSmsPart.getErrorString()
					  << ")";
	}
	else
	{
		aDebug.nospace() << "SmsPart(["
					  << aSmsPart.getPartNumber() << "/"
					  << aSmsPart.getPartCount() << "] "
					  << aSmsPart.getText() << ", "
					  << aSmsPart.getSenderAddress() << ", "
					  << aSmsPart.getDateTime()
						 .toString("dd.MM.yyyy hh:mm:ss") << ")";
	}



	return aDebug.space();
}

//------------------------------------------------------------------------------
QDebug operator<<(QDebug aDebug, SmsPart * aPart)
{
	if (!aPart->isValid())
	{
		aDebug.nospace() << "SmsPart(invalid message: "
					  << aPart->getErrorString() << ")";
	}
	else
	{
		aDebug.nospace() << "SmsPart(["
					  << aPart->getPartNumber() << "/"
					  << aPart->getPartCount() << "] "
					  << aPart->getText() << ", "
					  << aPart->getSenderAddress() << ", "
					  << aPart->getDateTime()
						 .toString("dd.MM.yyyy hh:mm:ss") << ")";
	}

	return aDebug.space();
}


//SmsPartData
//------------------------------------------------------------------------------
SmsPartData::SmsPartData() :
	mHasErrors(false), mMessageType(SmsPart::Unknown), mPartNumber(0),
	mHasUserDataHeader(false), mPartCount(1)
{
	mText = "[missing text]";
}

//------------------------------------------------------------------------------
SmsPartData::SmsPartData(const SmsPartData & aOther) :
	QSharedData(aOther)
{
	mPartId = aOther.mPartId;
	mPartNumber = aOther.mPartNumber;
	mPartCount = aOther.mPartCount;
	mSenderAddress = aOther.mSenderAddress;
	mDateTime = aOther.mDateTime;
	mMessageType = aOther.mMessageType;
	mCodingScheme = aOther.mCodingScheme;
	mText = aOther.mText;
	mHasErrors = aOther.mHasErrors;
	mErrorString = aOther.mErrorString;
	mHasUserDataHeader = aOther.mHasUserDataHeader;
	mUserDataHeaderLength = aOther.mUserDataHeaderLength;
}

//------------------------------------------------------------------------------
SmsPartData::~SmsPartData()
{
}
//------------------------------------------------------------------------------
