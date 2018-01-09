/* @file Протокол ccTalk. */

// STL
#include <numeric>

// Modules
#include "ccTalkConstants.h"

// Project
#include "ccTalk.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
CCTalkCAProtocol::CCTalkCAProtocol() : mAddress(CCCTalk::Address::Default), mCRCType(CCCTalk::ECRCType::CRC8)
{
}

//--------------------------------------------------------------------------------
void CCTalkCAProtocol::setAddress(uchar aAddress)
{
	mAddress = aAddress;
}

//--------------------------------------------------------------------------------
void CCTalkCAProtocol::setCRCType(CCCTalk::ECRCType::Enum aType)
{
	mCRCType = aType;
}

//--------------------------------------------------------------------------------
bool CCTalkCAProtocol::check(QByteArray & aAnswer)
{
	if (aAnswer.size() < CCCTalk::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("Too few bytes in answer = %1, need min = %2").arg(aAnswer.size()).arg(CCCTalk::MinAnswerSize));
		return false;
	}

	uchar destinationAddress = uchar(aAnswer[0]);

	if (destinationAddress != CCCTalk::Address::Host)
	{
		toLog(LogLevel::Error, QString("Wrong destination address = %1, need = %2").arg(destinationAddress).arg(CCCTalk::Address::Host));
		return false;
	}

	int length = aAnswer[1];
	int dataSize = aAnswer.size() - 5;

	if (length != dataSize)
	{
		toLog(LogLevel::Error, QString("Wrong length = %1, need = %2").arg(length).arg(dataSize));
		return false;
	}

	uchar deviceAddress = uchar(aAnswer[2]);

	if (deviceAddress != mAddress)
	{
		toLog(LogLevel::Error, QString("Wrong length = %1, need = %2").arg(deviceAddress).arg(mAddress));
		return false;
	}

	uchar header = uchar(aAnswer[3]);

	if (header && (header != CCCTalk::NAK))
	{
		toLog(LogLevel::Error, QString("Wrong header = %1, need = 0").arg(header));
		return false;
	}

	if (length)
	{
		QByteArray answer = aAnswer.left(aAnswer.size() - 1);
		uchar dataCRC = uchar(std::accumulate(answer.begin(), answer.end(), 0));
		uchar answerCRC = uchar(aAnswer[aAnswer.size() - 1]);

		if (uchar(dataCRC + answerCRC))
		{
			toLog(LogLevel::Error, QString("Wrong CRC = %1, need = %2.").arg(answerCRC).arg(uchar(0) - dataCRC));
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

	QByteArray request = aCommandData;
	request.prepend(CCCTalk::Address::Host);
	request.prepend(uchar(aCommandData.size() - 1));
	request.prepend(mAddress);
	request.append(-uchar(std::accumulate(request.begin(), request.end(), 0)));    // CRC 8 бит

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
			toLog(LogLevel::Normal, QString("%1 in answer, %2")
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
	while ((timer.elapsed() < CCCTalk::ReadingTimeout) && ((aAnswer.size() < (length + 5 + int(mCRCType == CCCTalk::ECRCType::CRC16))) || (length == -1)));

	toLog(LogLevel::Normal, QString("ccTalk: << {%2}").arg(aAnswer.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
