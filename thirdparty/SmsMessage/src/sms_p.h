/* @file Смс сообщение. */

#pragma once

// Qt
#include <QtCore/QDateTime>
#include <QtCore/QHash>

class SmsPart;

//------------------------------------------------------------------------------
class SmsData : public QSharedData
{
public:
	SmsData();
	SmsData(const SmsData & aOther);
	virtual ~SmsData();
	bool setPart(int aId, SmsPart aPart);
	QHash<int, SmsPart> mParts;
	int mPartCount;
	QString mSenderAddress;
};
//------------------------------------------------------------------------------
