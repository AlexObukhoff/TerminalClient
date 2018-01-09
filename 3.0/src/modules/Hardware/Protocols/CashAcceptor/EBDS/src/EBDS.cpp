/* @file Протокол EBDS. */

#include "EBDS.h"
#include "EBDSConstants.h"

//--------------------------------------------------------------------------------
EBDSProtocol::EBDSProtocol(): mACK(false)
{
}

//--------------------------------------------------------------------------------
const uchar EBDSProtocol::calcCRC(const QByteArray & aData)
{
	uchar sum = aData[1];

	for (int i = 2; i < aData.size() - 1; ++i)
	{
		sum ^= static_cast<uchar>(aData[i]);
	}

	return sum;
}

//--------------------------------------------------------------------------------
bool EBDSProtocol::check(const QByteArray & aCommandPacket, const QByteArray & aAnswerPacket)
{
	// проверяем первый байт
	if (aAnswerPacket[0] != CEBDS::Prefix)
	{
		toLog(LogLevel::Error, "EBDS: Invalid first byte (prefix).");
		return false;
	}

	// проверяем последний байт
	if (aAnswerPacket.right(2)[0] != CEBDS::Postfix)
	{
		toLog(LogLevel::Error, "EBDS: Invalid last byte (postfix).");
		return false;
	}

	// вытаскиваем и проверяем длину сообщения
	int length = aAnswerPacket[1];

	if (length != aAnswerPacket.size())
	{
		toLog(LogLevel::Error, "EBDS: Invalid length of the message.");
		return false;
	}

	// проверяем контрольную сумму
	if (calcCRC(aAnswerPacket.left(aAnswerPacket.size() - 1)) != aAnswerPacket[length - 1])
	{
		toLog(LogLevel::Error, "EBDS: Invalid CRC of the message");
		return false;
	}

	char commandACK = aCommandPacket[2] & CEBDS::ACKMask;
	char  answerACK =  aAnswerPacket[2] & CEBDS::ACKMask;

	// проверяем тип сообщения и ACK
	if (commandACK != answerACK)
	{
		toLog(LogLevel::Error, "EBDS: Invalid ACK of the message.");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
TResult EBDSProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData, bool aNeedAnswer)
{
	QByteArray request;

	request.append(CEBDS::Prefix);
	request.append(char(4 + aCommandData.size()));
	request.append(aCommandData[0] | char(mACK));
	request.append(aCommandData.mid(1));
	request.append(CEBDS::Postfix);
	request.append(calcCRC(request));

	toLog(LogLevel::Normal, QString("EBDS: >> {%1}").arg(request.toHex().data()));
	mACK = !mACK;

	if (!mPort->write(request))
	{
		return CommandResult::Port;
	}

	if (!aNeedAnswer)
	{
		return CommandResult::OK;
	}
	else if (!getAnswer(aAnswerData))
	{
		return CommandResult::Port;
	}
	else if (aAnswerData.isEmpty())
	{
		return CommandResult::NoAnswer;
	}
	else if (!check(request, aAnswerData))
	{
		return CommandResult::Protocol;
	}

	aAnswerData = aAnswerData.mid(2, aAnswerData.size() - 4);
	aAnswerData[0] = aAnswerData[0] & ~CEBDS::ACKMask;

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
const bool EBDSProtocol::getAnswer(QByteArray & aAnswer)
{
	aAnswer.clear();
	QByteArray data;
	uchar length = 0;

	QTime clockTimer;
	clockTimer.restart();

	do
	{
		data.clear();

		if (!mPort->read(data, 20))
		{
			return false;
		}

		aAnswer.append(data);

		if (aAnswer.size() > 1)
		{
			length = aAnswer[1];
		}
	}
	while ((clockTimer.elapsed() < CEBDS::AnswerTimeout) && ((aAnswer.size() < length) || !length));

	toLog(LogLevel::Normal, QString("EBDS: << {%1}").arg(aAnswer.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
