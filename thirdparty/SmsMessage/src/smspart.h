/* @file Часть смс сообщения. */

#pragma once

// Qt
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QScopedPointer>
#include <QtCore/QDebug>

class SmsPartData;

//------------------------------------------------------------------------------
class SmsPart
{
public:
	enum EMessageType
	{
		Deliver,
		StatusReport,
		SubmitReport,
		Unknown
	};

	SmsPart();
	SmsPart(const SmsPart & aOther);
    SmsPart(SmsPartData * d);
#ifdef Q_COMPILER_RVALUE_REFS
    inline SmsPart(SmsPart && aOther);
    inline SmsPart & operator=(SmsPart && aOther);
#endif
	virtual ~SmsPart();
	QString getSenderAddress() const;
	QString getText() const;
	QDateTime getDateTime() const;
	bool isValid() const;
	QString getErrorString() const;
	EMessageType getMessageType() const;
	int getPartNumber() const;
	int getPartCount() const;
	QByteArray getPartId() const;

private:
    QSharedDataPointer<SmsPartData> d;
};
//------------------------------------------------------------------------------
