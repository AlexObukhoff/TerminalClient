/* @file Протокол ФР ШтрихПэй. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTime>
#include <Common/QtHeadersEnd.h>

// Project
#include "ShtrihPayFR.h"
#include "ShtrihPayFRConstants.h"

//--------------------------------------------------------------------------------
ShtrihPayFRProtocol::ShtrihPayFRProtocol() : mPacketId(0)
{
}

//--------------------------------------------------------------------------------
ushort ShtrihPayFRProtocol::calcCRC(const QByteArray & aData)
{
	unsigned short CRC = 0xffff; // Init

	for (int i = 0; i < aData.size(); ++i)
	{
		unsigned short newCRC = uchar((CRC >> 8) | (CRC << 8));
		newCRC ^= aData[i];
		newCRC ^= uchar((newCRC & 0x00ff) >> 4);
		newCRC ^= newCRC << 12;
		newCRC ^= (newCRC & 0x00ff) << 5;
		CRC = newCRC;
	}

	return CRC;
}

//--------------------------------------------------------------------------------
bool ShtrihPayFRProtocol::unpackData(const QByteArray & aPacket, QByteArray & aData)
{
	if (aPacket.size() < CShtrihPayFR::MinPacketAnswerSize)
	{
		toLog(LogLevel::Error, QString("ShtrihPay: The length of the packet is less than %1 byte").arg(CShtrihPayFR::MinPacketAnswerSize));
		return false;
	}

	if (aPacket[0] != CShtrihPayFR::Prefix)
	{
		toLog(LogLevel::Error, "ShtrihPay: Invalid first byte (prefix).");
		return false;
	}

	int length = aPacket[1];

	if (length < (aPacket.size() - 3))
	{
		toLog(LogLevel::Error, "ShtrihPay: Invalid length of the packet");
		return false;
	}

	QByteArray trimmedAnswer = aPacket.mid(1, length + 1);

	uchar localCRC = static_cast<uchar>(calcCRC(trimmedAnswer));
	uchar CRC = aPacket[length + 2];

	if (localCRC != CRC)
	{
		toLog(LogLevel::Error, "ShtrihPay: Invalid CRC of the answer message.");
		return false;
	}

	// записываем данные
	aData = trimmedAnswer.mid(1, length);

	return true;
}

//--------------------------------------------------------------------------------
bool ShtrihPayFRProtocol::openSession()
{
	int count = 0;

	do
	{
		if (count++)
		{
			toLog(LogLevel::Normal, "ShtrihPay: session opening, attempt #" + QString::number(count));
		}

		if (!sendENQ())
		{
			return false;
		}

		QByteArray answerData;

		if (!mPort->read(answerData, CShtrihPayFR::Timeouts::ENQAnswer))
		{
			return false;
		}

		if (answerData.startsWith(ASCII::NAK))
		{
			return true;
		}

		if (answerData.startsWith(ASCII::ACK))
		{
			toLog(LogLevel::Normal, "ShtrihPay: Device returns ACK, getting the last answer and sending ACK");

			QTime timer;
			timer.start();

			while ((uint(answerData.size()) < (uint(answerData[2]) + 4)) && (timer.elapsed() < CShtrihPayFR::Timeouts::DefaultAnswer))
			{
				QByteArray data;

				if(!mPort->read(data))
				{
					return false;
				}

				answerData.append(data);
			}

			if (!sendACK())
			{
				return false;
			}
		}
		else if (!answerData.isEmpty())
		{
			toLog(LogLevel::Error, QString("ShtrihPay: Device returns unknown answer {%1}").arg(answerData.toHex().data()));
		}
	}
	while (count < CShtrihPayFR::MaxServiceRequests);

	toLog(LogLevel::Error, "ShtrihPay: Failed to open session");

	return false;
}

//--------------------------------------------------------------------------------
TResult ShtrihPayFRProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData, int aTimeout)
{
	if (aCommandData.isEmpty())
	{
		toLog(LogLevel::Error, "ShtrihPay: Command packet is empty");
		return CommandResult::Driver;
	}

	QByteArray commandData;

	int length = aCommandData.size() + 2;
	commandData.append(char(length));
	commandData.append(char(length >> 8));

	mPacketId += mPacketId ? 1 : 2;
	commandData.append(char(mPacketId));
	commandData.append(char(mPacketId >> 8));

	commandData.append(aCommandData);

	ushort CRC = calcCRC(commandData);
	commandData.append(char(CRC));
	commandData.append(char(CRC >> 8));

	commandData.prepend(CShtrihPayFR::Prefix);

	int repeatCount = 0;

	do
	{
		toLog(LogLevel::Normal, QString("ShtrihPay: >> {%1} - iteration %2").arg(commandData.toHex().data()).arg(repeatCount + 1));
		QByteArray answer;

		if (!repeatCount)
		{
			if (!openSession())
			{
				return CommandResult::Transport;
			}

			if (!execCommand(commandData, answer, aTimeout))
			{
				toLog(LogLevel::Error, "ShtrihPay: Failed to execute command");
				return CommandResult::Transport;
			}
		}
		else if (!regetAnswer(answer))
		{
			return CommandResult::Transport;
		}

		toLog(LogLevel::Normal, QString("ShtrihPay: << {%1}").arg(answer.toHex().data()));
		QByteArray unpackedAnswer;

		if (unpackData(answer, unpackedAnswer))
		{
			if (!sendACK())
			{
				return CommandResult::Transport;
			}

			aAnswerData = unpackedAnswer;

			return CommandResult::OK;
		}

		toLog(LogLevel::Error, "ShtrihPay: Failed to unpack data");

		repeatCount++;
	}
	while(repeatCount < CShtrihPayFR::MaxRepeatPacket);

	return CommandResult::Protocol;
}

//--------------------------------------------------------------------------------
bool ShtrihPayFRProtocol::execCommand(const QByteArray & aCommandPacket, QByteArray & aAnswerData, int aTimeout)
{
	int countRequest = 0;

	do
	{
		if (!mPort->write(aCommandPacket))
		{
			return false;
		}

		if (!getAnswer(aAnswerData, aTimeout))
		{
			toLog(LogLevel::Error, "ShtrihPay: Failed to get answer");
			return false;
		}

		if (!aAnswerData.isEmpty() && (aAnswerData[0] == ASCII::NAK))
		{
			toLog(LogLevel::Warning, "ShtrihPay: Answer contains NAK, iteration " + QString::number(countRequest + 1));
			countRequest++;
		}
		else
		{
			return true;
		}
	}
	while(countRequest < CShtrihPayFR::MaxRepeatPacket);

	return false;
}

//--------------------------------------------------------------------------------
bool ShtrihPayFRProtocol::regetAnswer(QByteArray & aAnswerData)
{
	if (!sendNAK() || !sendENQ())
	{
		return false;
	}

	if (!getAnswer(aAnswerData, CShtrihPayFR::Timeouts::DefaultAnswer))
	{
		toLog(LogLevel::Error, "ShtrihPay: Failed to get answer");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool ShtrihPayFRProtocol::getAnswer(QByteArray & aData, int aTimeout)
{
	QByteArray answerData;

	bool ACKreceived = false;

	QTime clockTimer;
	clockTimer.start();
	uchar length = 0;

	while (clockTimer.elapsed() < aTimeout)
	{
		answerData.clear();

		if (!mPort->read(answerData))
		{
			return false;
		}

		if (!answerData.isEmpty())
		{
			if (!ACKreceived)
			{
				if (answerData[0] == ASCII::ACK)
				{
					answerData.remove(0, 1);
					ACKreceived = true;
				}
				else if (answerData[0] == ASCII::NAK)
				{
					aData = answerData.left(1);
					return true;
				}
			}

			aData.push_back(answerData);

			if (!length && (aData.size() > 2))
			{
				length = aData[1] + 3;
			}

			if (length && (aData.size() >= length))
			{
				return true;
			}
		}
	}

	return false;
}

//--------------------------------------------------------------------------------
bool ShtrihPayFRProtocol::sendACK()
{
	if (!mPort->write(QByteArray(1, ASCII::ACK)))
	{
		toLog(LogLevel::Error, "ShtrihPay: Failed to send ACK packet");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool ShtrihPayFRProtocol::sendNAK()
{
	if (!mPort->write(QByteArray(1, ASCII::NAK)))
	{
		toLog(LogLevel::Error, "ShtrihPay: Failed to send NAK");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool ShtrihPayFRProtocol::sendENQ()
{
	if (!mPort->write(QByteArray(1, ASCII::ENQ)))
	{
		toLog(LogLevel::Error, "ShtrihPay: Failed to send ENQ");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
