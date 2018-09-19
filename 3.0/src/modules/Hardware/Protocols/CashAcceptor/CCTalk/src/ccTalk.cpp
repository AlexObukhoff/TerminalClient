/* @file Протокол ccTalk. */

// STL
#include <numeric>

// Modules
#include "Hardware/Common/HardwareConstants.h"
#include "ccTalkConstants.h"

// Project
#include "ccTalk.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
CCTalkCAProtocol::CCTalkCAProtocol() : mAddress(CCCTalk::Address::Unknown), mType(CCCTalk::UnknownType)
{
}

//--------------------------------------------------------------------------------
void CCTalkCAProtocol::setAddress(uchar aAddress)
{
	if (mAddress != aAddress)
	{
		toLog(LogLevel::Normal, "ccTalk: Set device address " + QString::number(aAddress));

		mAddress = aAddress;
	}
}

//--------------------------------------------------------------------------------
void CCTalkCAProtocol::setType(const QString & aType)
{
	if ((mType != aType) && CCCTalk::ProtocolTypes.contains(aType))
	{
		toLog(LogLevel::Normal, "ccTalk: Set protocol type " + aType);

		mType = aType;
	}
}

//--------------------------------------------------------------------------------
uchar CCTalkCAProtocol::calcCRC8(const QByteArray & aData)
{
	return -uchar(std::accumulate(aData.begin(), aData.end(), 0));
}

//--------------------------------------------------------------------------------
ushort CCTalkCAProtocol::calcCRC16(const QByteArray & aData)
{
	ushort CRC = 0;

	for (int i = 0; i < aData.size(); ++i)
	{
		CRC ^= (aData[i] << 8);

		for (int j = 0; j < 8; ++j)
		{
			if (CRC & CCCTalk::LastBit)
			{
				CRC = (CRC << 1) ^ CCCTalk::Polynominal;
			}
			else
			{
				CRC <<= 1;
			}
		}
	}

	return CRC;
}

//--------------------------------------------------------------------------------
bool CCTalkCAProtocol::check(QByteArray & aAnswer)
{
	if (aAnswer.size() < CCCTalk::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("ccTalk: Too few bytes in answer = %1, need min = %2").arg(aAnswer.size()).arg(CCCTalk::MinAnswerSize));
		return false;
	}

	char destinationAddress = aAnswer[0];

	if (destinationAddress != CCCTalk::Address::Host)
	{
		toLog(LogLevel::Error, QString("ccTalk: Wrong destination address = %1, need = %2").arg(uchar(destinationAddress)).arg(uchar(CCCTalk::Address::Host)));
		return false;
	}

	int length = aAnswer[1];
	int dataSize = aAnswer.size() - 5;

	if (length != dataSize)
	{
		toLog(LogLevel::Error, QString("ccTalk: Wrong length = %1, need = %2").arg(length).arg(dataSize));
		return false;
	}

	if (mType == CHardware::CashAcceptor::CCTalkTypes::CRC8)
	{
		char deviceAddress = aAnswer[2];

		if (deviceAddress != mAddress)
		{
			toLog(LogLevel::Error, QString("ccTalk: Wrong address = %1, need = %2").arg(uchar(deviceAddress)).arg(uchar(mAddress)));
			return false;
		}
	}

	uchar header = uchar(aAnswer[3]);

	if (header && (header != CCCTalk::NAK))
	{
		toLog(LogLevel::Error, QString("ccTalk: Wrong header = %1, need = 0").arg(header));
		return false;
	}

	if (mType == CHardware::CashAcceptor::CCTalkTypes::CRC8)
	{
		QByteArray answer = aAnswer.left(aAnswer.size() - 1);
		uchar dataCRC = calcCRC8(answer);
		uchar answerCRC = uchar(aAnswer[aAnswer.size() - 1]);

		if (dataCRC != answerCRC)
		{
			toLog(LogLevel::Error, QString("ccTalk: Wrong CRC = %1, need = %2").arg(answerCRC).arg(uchar(uchar(0) - dataCRC)));
			return false;
		}
	}
	else if (mType == CHardware::CashAcceptor::CCTalkTypes::CRC16)
	{
		QByteArray answer = aAnswer.left(2) + aAnswer.mid(3, aAnswer.size() - 4);
		ushort dataCRC = calcCRC16(answer);
		ushort answerCRC = uchar(aAnswer[2]) | ushort(aAnswer[aAnswer.size() - 1] << 8);

		if (dataCRC != answerCRC)
		{
			toLog(LogLevel::Error, QString("ccTalk: Wrong CRC = %1, need = %2").arg(ProtocolUtils::toHexLog(answerCRC)).arg(ProtocolUtils::toHexLog(dataCRC)));
			return false;
		}
	}

	aAnswer = aAnswer.mid(3, 1 + aAnswer[1]);

	return true;
}

//---------------------------------------------------------------------------------
TResult CCTalkCAProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData)
{
	aAnswerData.clear();

	if (mAddress == CCCTalk::Address::Unknown)
	{
		toLog(LogLevel::Error, "ccTalk: Failed to process command due to address is not set");
		return CommandResult::Driver;
	}

	QByteArray request = aCommandData;
	uchar size = uchar(aCommandData.size() - 1);

	if (mType == CHardware::CashAcceptor::CCTalkTypes::CRC8)
	{
		request.prepend(CCCTalk::Address::Host);
		request.prepend(size);
		request.prepend(mAddress);
		request.append(calcCRC8(request));
	}
	else if (mType == CHardware::CashAcceptor::CCTalkTypes::CRC16)
	{
		request.prepend(size);
		request.prepend(mAddress);

		ushort CRC = calcCRC16(request);
		request.insert(2, uchar(CRC));
		request.append(uchar(CRC >> 8));
	}
	else
	{
		toLog(LogLevel::Error, "ccTalk: Failed to process command due to unknown ccTalk type " + mType);
		return CommandResult::Driver;
	}

	bool nak  = false;
	bool busy = false;
	int busyNAKRepeat = 0;

	do
	{
		if (busyNAKRepeat)
		{
			SleepHelper::msleep(CCCTalk::Timeouts::NAKBusy);
		}

		toLog(LogLevel::Normal, QString("ccTalk: >> {%2}").arg(request.toHex().data()));

		if (!mPort->write(request) || !getAnswer(aAnswerData, request))
		{
			return CommandResult::Port;
		}

		if (aAnswerData.isEmpty())
		{
			return CommandResult::NoAnswer;
		}
		else if (!check(aAnswerData))
		{
			return CommandResult::Protocol;
		}

		nak  = (aAnswerData.size() == 1) && (aAnswerData[0] == CCCTalk::NAK);
		busy = (aAnswerData.size() == 1) && (aAnswerData[0] == CCCTalk::BUSY);

		if (nak || busy)
		{
			toLog(LogLevel::Normal, QString("ccTalk: %1 in answer, %2")
				.arg(nak ? "NAK" : "BYSY")
				.arg((busyNAKRepeat <= CCCTalk::MaxBusyNAKRepeats) ? "repeat sending the messsage" : "cancel sending!"));
		}
	}
	while((busy || nak) && (++busyNAKRepeat < CCCTalk::MaxBusyNAKRepeats));

	return (!busy && !nak) ? CommandResult::OK : CommandResult::Transport;
}

//--------------------------------------------------------------------------------
bool CCTalkCAProtocol::getAnswer(QByteArray & aAnswer, const QByteArray & aCommandData)
{
	aAnswer.clear();

	QByteArray data;
	int length = -1;
	QTime timer;
	timer.start();

	do
	{
		if (!mPort->read(data))
		{
			return CommandResult::Port;
		}

		aAnswer.append(data);

		for(int i = 0; i < 2; ++i)
		{
			length = -1;

			if (aAnswer.size() >= 2)
			{
				length = aAnswer[1];

				if (aAnswer.startsWith(aCommandData))
				{
					aAnswer = aAnswer.mid(aCommandData.size());
				}
			}
		}
	}
	while ((timer.elapsed() < CCCTalk::Timeouts::Reading) && ((aAnswer.size() < (length + 5)) || (length == -1)));

	toLog(LogLevel::Normal, QString("ccTalk: << {%2}").arg(aAnswer.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
