/* @file Протокол ФР Штрих. */

#include "Puloon.h"
#include "PuloonConstants.h"

//--------------------------------------------------------------------------------
uchar Puloon::calcCRC(const QByteArray & aData) const
{
	if (!aData.size())
	{
		return 0;
	}

	uchar sum = 0;

	for (int i = 0; i < aData.size(); ++i)
	{
		sum ^= static_cast<uchar>(aData[i]);
	}

	return sum;
}

//--------------------------------------------------------------------------------
void Puloon::pack(const QByteArray & aCommandPacket, QByteArray & aPacket) const
{
	aPacket.append(CPuloon::CommandMark);
	aPacket.append(aCommandPacket);
	aPacket.insert(2, CPuloon::Prefix);
	aPacket.append(CPuloon::Postfix);
	aPacket.append(calcCRC(aPacket));
}

//--------------------------------------------------------------------------------
bool Puloon::check(const QByteArray & aAnswer, const QByteArray & aRequest) const
{
	if (aAnswer.size() < CPuloon::MinPacketAnswerSize)
	{
		toLog(LogLevel::Error, QString("Puloon: The length of the packet is less than %1 byte").arg(CPuloon::MinPacketAnswerSize));
		return false;
	}

	if (aAnswer[0] != CPuloon::AnswerMark)
	{
		toLog(LogLevel::Error, QString("Puloon: Invalid answer mark = %1, need %2")
			.arg(ProtocolUtils::toHexLog<char>(aAnswer[0])).arg(ProtocolUtils::toHexLog(CPuloon::AnswerMark)));
		return false;
	}

	if (aAnswer[1] != aRequest[1])
	{
		toLog(LogLevel::Error, QString("Puloon: Invalid communication Id = %1, need %2")
			.arg(ProtocolUtils::toHexLog<char>(aAnswer[1])).arg(ProtocolUtils::toHexLog<char>(aRequest[1])));
		return false;
	}
	
	if (aAnswer[2] != CPuloon::Prefix)
	{
		toLog(LogLevel::Error, QString("Puloon: Invalid first byte (prefix) = %1, need %2")
			.arg(ProtocolUtils::toHexLog<char>(aAnswer[2])).arg(ProtocolUtils::toHexLog(CPuloon::Prefix)));
		return false;
	}

	if (aAnswer[3] != aRequest[3])
	{
		toLog(LogLevel::Error, QString("Puloon: Invalid answer command = %1, need = %2.")
			.arg(ProtocolUtils::toHexLog<char>(aAnswer[3])).arg(ProtocolUtils::toHexLog<char>(aRequest[3])));
		return false;
	}

	uchar answerCRC = calcCRC(aAnswer.left(aAnswer.size() - 1));
	uchar CRC = aAnswer[aAnswer.size() - 1];

	if (answerCRC != CRC)
	{
		toLog(LogLevel::Error, QString("Puloon: Invalid CRC = %1, need %2").arg(ProtocolUtils::toHexLog(answerCRC)).arg(ProtocolUtils::toHexLog(CRC)));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
TResult Puloon::processCommand(const QByteArray & aCommandData, TAnswerList & aAnswerList, int aTimeout) const
{
	if (aCommandData.isEmpty())
	{
		toLog(LogLevel::Error, "Puloon: Command packet is empty");
		return CommandResult::Driver;
	}

	QByteArray request;
	pack(aCommandData, request);

	int repeatCount = 0;

	do
	{
		QString logIteration = repeatCount ? QString(" - iteration %1").arg(repeatCount + 1) : "";
		toLog(LogLevel::Normal, QString("Puloon: >> {%1}%2").arg(request.toHex().data()).arg(logIteration));

		TAnswerList answerList;

		if (!repeatCount)
		{
			if (!execCommand(request, answerList, aTimeout))
			{
				toLog(LogLevel::Error, "Puloon: Failed to execute command");
				return CommandResult::Transport;
			}
		}
		else if (!regetAnswer(answerList))
		{
			return CommandResult::Transport;
		}

		if (check(answerList.last(), request))
		{
			if (!mPort->write(QByteArray(1, ASCII::ACK)))
			{
				toLog(LogLevel::Error, "Puloon: Failed to send ACK packet");
				return CommandResult::Transport;
			}

			aAnswerList = answerList;

			if (!aAnswerList.isEmpty())
			{
				QByteArray & lastAnswer = aAnswerList.last();

				if (lastAnswer.size() > CPuloon::ItemDataSize)
				{
					lastAnswer = lastAnswer.mid(4, lastAnswer.size() - 6);
				}
			}

			return CommandResult::OK;
		}

		toLog(LogLevel::Error, "Puloon: Failed to unpack data");

		repeatCount++;
	}
	while(repeatCount < CPuloon::MaxRepeatPacket);

	return CommandResult::Protocol;
}

//--------------------------------------------------------------------------------
bool Puloon::execCommand(const QByteArray & aCommandPacket, TAnswerList & aAnswerList, int aTimeout) const
{
	int countRequest = 0;

	do
	{
		if (!mPort->write(aCommandPacket))
		{
			return false;
		}

		aAnswerList.clear();

		if (!getAnswer(aAnswerList, aTimeout))
		{
			return false;
		}

		if (!aAnswerList.isEmpty() && !aAnswerList[0].isEmpty() && (aAnswerList[0][0] == ASCII::NAK))
		{
			toLog(LogLevel::Warning, "Puloon: Answer contains NAK, iteration " + QString::number(countRequest + 1));
			countRequest++;
		}
		else
		{
			return true;
		}
	}
	while(countRequest < CPuloon::MaxRepeatPacket);

	return false;
}

//--------------------------------------------------------------------------------
bool Puloon::regetAnswer(TAnswerList & aAnswerList) const
{
	if (!mPort->write(QByteArray(1, ASCII::NAK)))
	{
		toLog(LogLevel::Error, "Puloon: Failed to send NAK");
		return false;
	}

	if (!getAnswer(aAnswerList, CPuloon::DefaultAnswerTimeout))
	{
		toLog(LogLevel::Error, "Puloon: Failed to reget answer");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool Puloon::getAnswer(TAnswerList & aAnswerList, int aTimeout) const
{
	QByteArray answerData;

	bool ACKreceived = false;

	QTime clockTimer;
	clockTimer.start();

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
					answerData = answerData.left(1);
					aAnswerList.clear();
					aAnswerList.append(answerData);

					return true;
				}
			}

			aAnswerList.isEmpty() ? aAnswerList.append(answerData) : aAnswerList.last().append(answerData);
			QByteArray lastData = aAnswerList.takeLast();

			while(!lastData.isEmpty())
			{
				int size = lastData.size();

				if ((size > CPuloon::ItemDataSize) && (lastData[0] == CPuloon::AnswerMark) && (lastData[CPuloon::ItemDataSize] == CPuloon::AnswerMark))
				{
					size = CPuloon::ItemDataSize;
				}

				aAnswerList.append(lastData.left(size));
				lastData = lastData.mid(size);
			}

			if (!aAnswerList.isEmpty())
			{
				QByteArray data = aAnswerList.last();
				int size = data.size();

				if ((size > 1) && (data[size - 2] == CPuloon::Postfix))
				{
					break;
				}
			}
		}
	}

	QByteArray fullData;

	foreach (const QByteArray & data, aAnswerList)
	{
		fullData += data;
		toLog(LogLevel::Normal, QString("Puloon: << {%1}").arg(data.toHex().data()));
	}

	return !fullData.isEmpty();
}

//--------------------------------------------------------------------------------
