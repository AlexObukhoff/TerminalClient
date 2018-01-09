/* @file Кардридер ACS ACR120. */

// ACR120 SDK
#include <ACR120U/include/ACR120U.h>

// Project
#include "ACR120.h"

//--------------------------------------------------------------------------------
ACR120::ACR120(): mHandle(0), mCardPresent(false)
{
	mPollingInterval = 500;
}

//--------------------------------------------------------------------------------
bool ACR120::isConnected()
{
	for (qint16 i = ACR120_USB1; i <= ACR120_USB8; ++i)
	{
		mHandle = ACR120_Open(i);

		if (mHandle > 0)
		{
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------
bool ACR120::release()
{
	bool result = TPollingHID::release();

	if (mHandle)
	{
		ACR120_Close(mHandle);

		mHandle = 0;
	}

	return result;
}

//--------------------------------------------------------------------------------
bool ACR120::getStatus(TStatusCodes & /*aStatusCodes*/)
{
	if (mHandle <= 0)
	{
		return false;
	}

	quint8 tagFound;
	quint8 tagType[4];
	quint8 tagLength[4];
	quint8 tagSerialNumber[4][10];

	qint16 status = ACR120_ListTags(mHandle, &tagFound, tagType, tagLength, tagSerialNumber);

	if (status == SUCCESS_READER_OP && tagFound && !mCardPresent)
	{
		mCardPresent = true;
		QByteArray rawData((const char *)(&tagSerialNumber[0][0]), tagLength[0]);

		QString receivedData = rawData.toHex().toUpper();

		if (!rawData.isEmpty())
		{
			toLog(LogLevel::Normal, QString("Scanner ACR120: data received: %1 (%2)")
				.arg(QString(rawData))
				.arg(receivedData));

			QVariantMap result;
			result[CHardwareSDK::HID::Text] = receivedData;

			emit data(result);
		}
	}
	else if (!tagFound && mCardPresent)
	{
		mCardPresent = false;
	}

	return true;
}

//--------------------------------------------------------------------------------
