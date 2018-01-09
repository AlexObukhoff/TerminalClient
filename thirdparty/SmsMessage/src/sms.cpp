/* @file Смс сообщение. */


// Проект
#include "sms.h"
#include "sms_p.h"
#include "smspart.h"
#include "smspart_p.h"
#include "smspdudecoder.h"

//------------------------------------------------------------------------------
Sms::Sms() :
	d(new SmsData)
{

}

//------------------------------------------------------------------------------
Sms::Sms(const Sms & aOther) :
	d(aOther.d)
{
}

#ifdef Q_COMPILER_RVALUE_REFS
//------------------------------------------------------------------------------
inline Sms::Sms(Sms && aOther)
    : d(aOther.d)
{
    aOther.d = 0;
}

//------------------------------------------------------------------------------
inline Sms & Sms::operator=(Sms && aOther)
{
    qSwap(d, aOther.d);
    return *this;
}
#endif

//------------------------------------------------------------------------------
Sms::~Sms()
{
}

//------------------------------------------------------------------------------
void Sms::addPart(const SmsPart & aPart)
{
	if (!d->mPartCount && aPart.isValid())
	{
		d->mPartCount = aPart.getPartCount();
		d->mSenderAddress = aPart.getSenderAddress();
	}

	d->mParts[aPart.getPartNumber()] = aPart;
}

//------------------------------------------------------------------------------
SmsPart Sms::getPart(int aId) const
{
    return d->mParts.value(aId);
}

//------------------------------------------------------------------------------
int Sms::getSize() const
{
	return d->mPartCount;
}

//------------------------------------------------------------------------------
QString Sms::getSenderAddress() const
{
	return d->mSenderAddress;
}

//------------------------------------------------------------------------------
QDateTime Sms::getDateTime() const
{
	SmsPart part = getPart(0);

	return part.isValid() ? part.getDateTime() : QDateTime();
}

//------------------------------------------------------------------------------
QString Sms::getText() const
{
	QString textMessage;
	for (int i = 0; i < getSize(); i++)
	{
		SmsPart part = getPart(i);
		textMessage += part.isValid() ? part.getText() : "[missing part]";
	}

	return textMessage;
}

//------------------------------------------------------------------------------
bool Sms::isValid() const
{
	for (int i = 0; i < getSize(); i++)
	{
		if (!getPart(i).isValid())
		{
			return false;
		}
	}

	return true;
}

//------------------------------------------------------------------------------
QString Sms::getErrorString() const
{
	QString s = "Sms(";

	for (int i = 0; i < getSize(); i++)
	{
		SmsPart part = getPart(i);

		if (!part.isValid())
		{
			s += QString("[invalid part %1: %2]")
				.arg(part.getPartNumber())
				.arg(part.getErrorString());
		}
	}

	s += ")";

	return s;
}

//------------------------------------------------------------------------------
SmsPart Sms::operator[](int aId) const
{
    return d->mParts.value(aId);
}

//------------------------------------------------------------------------------
SmsPart Sms::decode(const QString & aData)
{
	SmsPduDecoder decoder;
    return decoder.decodeMessage(QByteArray::fromHex(aData.toLatin1()));
}

//------------------------------------------------------------------------------
QList<Sms> Sms::join(QList<SmsPart> aParts)
{
	QList<Sms> list;
	QHash<QByteArray, Sms> messages;

	foreach (auto part, aParts)
	{
		if (part.getPartCount() == 1)
		{
			Sms message;
			message.addPart(part);
			list << message;
		}
		else
		{
			if (messages.contains(part.getPartId()))
			{
				messages[part.getPartId()].addPart(part);
			}
			else
			{
				Sms message;
				message.addPart(part);
				messages[part.getPartId()] = message;
			}
		}
	}

	list << messages.values();

	return list;
}

//------------------------------------------------------------------------------
QDebug operator<<(QDebug aDebug, const Sms & aSms)
{
	if (aSms.isValid())
	{
		aDebug.nospace()
			<< "Sms("
			<< aSms.getSize() << ", "
			<< aSms.getText() << ", "
			<< aSms.getSenderAddress() << ", "
			<< aSms.getDateTime().toString("dd.MM.yyyy hh:mm:ss") << ")";
	}
	else
	{
		aDebug.nospace()
			<< "Sms("
			<< aSms.getErrorString()
			<< ")";
	}

	return aDebug.space();
}

//------------------------------------------------------------------------------
QDebug operator<<(QDebug aDebug, Sms * aSms)
{
	aDebug.nospace() << aSms->getText();
	return aDebug.space();
}


//SmsData
//------------------------------------------------------------------------------
SmsData::SmsData() :
	mPartCount(0)
{
}

//------------------------------------------------------------------------------
SmsData::SmsData(const SmsData & aOther) :
	QSharedData(aOther),
	mPartCount(aOther.mPartCount),
	mSenderAddress(aOther.mSenderAddress),
	mParts(aOther.mParts)
{
}

//------------------------------------------------------------------------------
SmsData::~SmsData()
{
}

//------------------------------------------------------------------------------
bool SmsData::setPart(int aId, SmsPart aPart)
{
	if (mParts.contains(aId))
	{
		mParts[aId] = aPart;
		return true;
	}

	return false;
}
//------------------------------------------------------------------------------
