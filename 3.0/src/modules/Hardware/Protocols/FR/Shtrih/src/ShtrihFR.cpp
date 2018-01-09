/* @file Протокол ФР Штрих. */

#include "ShtrihFR.h"
#include "ShtrihFRConstants.h"

//--------------------------------------------------------------------------------
uchar ShtrihFRProtocol::calcCRC(const QByteArray & aData)
{
	if (!aData.size())
	{
		return 0;
	}

	uchar sum = aData[0];

	for (int i = 1; i < aData.size(); ++i)
	{
		sum ^= static_cast<uchar>(aData[i]);
	}

	return sum;
}

//--------------------------------------------------------------------------------
bool ShtrihFRProtocol::check(const QByteArray & aCommand)
{
	if (aCommand.size() < CShtrihFR::MinPacketAnswerSize)
	{
		toLog(LogLevel::Error, QString("Shtrih: Invalid answer length = %1, need %2 minimum").arg(aCommand.size()).arg(CShtrihFR::MinPacketAnswerSize));
		return false;
	}

	using namespace ProtocolUtils;

	if (aCommand[0] != CShtrihFR::Prefix)
	{
		toLog(LogLevel::Error, QString("Shtrih: Invalid prefix = %1, need %2").arg(toHexLog(aCommand[0])).arg(toHexLog(CShtrihFR::Prefix)));
		return false;
	}

	int length = int(uchar(aCommand[1])) + 3;
	int answerLength = aCommand.size();

	if (length > answerLength)
	{
		toLog(LogLevel::Error, QString("Shtrih: Invalid length of the answer = %1, need %2").arg(answerLength).arg(length));
		return false;
	}

	uchar CRC = uchar(calcCRC(aCommand.mid(1, length - 2)));
	uchar answerCRC = aCommand[length - 1];

	if (answerCRC != CRC)
	{
		toLog(LogLevel::Error, QString("Shtrih: Invalid CRC of the answer = %1, need %2").arg(toHexLog(answerCRC)).arg(toHexLog(CRC)));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool ShtrihFRProtocol::readData(QByteArray & aData, int aTimeout)
{
	QByteArray data;

	if (!mPort->read(data, aTimeout))
	{
		return false;
	}

	int index = ((mPort->getType() == SDK::Driver::EPortTypes::TCP) && aData.isEmpty() && data.startsWith(CShtrihFR::Trash)) ? 1 : 0;
	aData += data.mid(index);

	return true;
}

//--------------------------------------------------------------------------------
bool ShtrihFRProtocol::openSession()
{
	int count = 0;

	do
	{
		if (count++)
		{
			toLog(LogLevel::Normal, "Shtrih: session opening, attempt #" + QString::number(count));
		}

		if (!sendENQ())
		{
			return false;
		}

		QByteArray answerData;

		if (!readData(answerData, CShtrihFR::Timeouts::ENQAnswer))
		{
			return false;
		}

		if (answerData.startsWith(ASCII::NAK))
		{
			return true;
		}

		if (answerData.startsWith(ASCII::ACK))
		{
			toLog(LogLevel::Normal, "Shtrih: Device returns ACK, getting the last answer and sending ACK");

			QTime timer;
			timer.start();

			while ((uint(answerData.size()) < (uint(answerData[2]) + 4)) && (timer.elapsed() < CShtrihFR::Timeouts::DefaultAnswer))
			{
				if (!readData(answerData))
				{
					return false;
				}
			}

			if (!sendACK())
			{
				return false;
			}
		}
		else if (!answerData.isEmpty())
		{
			toLog(LogLevel::Error, QString("Shtrih: Device returns unknown answer {%1}").arg(answerData.toHex().data()));
		}
	}
	while (count < CShtrihFR::MaxServiceRequests);

	toLog(LogLevel::Error, "Shtrih: Failed to open session");

	return false;
}

//--------------------------------------------------------------------------------
TResult ShtrihFRProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData, int aTimeout)
{
	if (aCommandData.isEmpty())
	{
		toLog(LogLevel::Error, "Shtrih: Command packet is empty");
		return CommandResult::Driver;
	}

	QByteArray command;
	command.append(uchar(aCommandData.size()));
	command.append(aCommandData);
	command.append(calcCRC(command));
	command.prepend(CShtrihFR::Prefix);

	int repeatCount = 0;

	do
	{
		QString logIteration = repeatCount ? QString(" - iteration %1").arg(repeatCount + 1) : "";
		toLog(LogLevel::Normal, QString("Shtrih: >> {%1}%2").arg(command.toHex().data()).arg(logIteration));

		QByteArray answer;

		if (!repeatCount)
		{
			if (!openSession())
			{
				return CommandResult::Transport;
			}

			if (!execCommand(command, answer, aTimeout))
			{
				toLog(LogLevel::Error, "Shtrih: Failed to execute command");
				return CommandResult::Transport;
			}
		}
		else if (!regetAnswer(answer))
		{
			return CommandResult::Transport;
		}

		toLog(LogLevel::Normal, QString("Shtrih: << {%1}").arg(answer.toHex().data()));

		if (check(answer))
		{
			aAnswerData = answer.mid(2, uchar(answer[1]));

			if (!sendACK())
			{
				return CommandResult::Transport;
			}

			return CommandResult::OK;
		}

		repeatCount++;
	}
	while(repeatCount < CShtrihFR::MaxRepeatPacket);

	return CommandResult::Protocol;
}

//--------------------------------------------------------------------------------
bool ShtrihFRProtocol::execCommand(const QByteArray & aCommand, QByteArray & aAnswer, int aTimeout)
{
	int countRequest = 0;

	do
	{
		if (!mPort->write(aCommand))
		{
			return false;
		}

		if (!getAnswer(aAnswer, aTimeout))
		{
			return false;
		}

		if (!aAnswer.isEmpty() && (aAnswer[0] == ASCII::NAK))
		{
			toLog(LogLevel::Warning, "Shtrih: Answer contains NAK, iteration " + QString::number(countRequest + 1));
			countRequest++;
		}
		else
		{
			return true;
		}
	}
	while(countRequest < CShtrihFR::MaxRepeatPacket);

	return false;
}

//--------------------------------------------------------------------------------
bool ShtrihFRProtocol::regetAnswer(QByteArray & aAnswerData)
{
	if (!sendNAK() || !sendENQ())
	{
		return false;
	}

	return getAnswer(aAnswerData, CShtrihFR::Timeouts::DefaultAnswer);
}

//--------------------------------------------------------------------------------
bool ShtrihFRProtocol::getAnswer(QByteArray & aData, int aTimeout)
{
	QTime clockTimer;
	clockTimer.start();
	uchar length = 0;

	aData.clear();

	while (clockTimer.elapsed() < aTimeout)
	{
		QByteArray data;

		if (!readData(data))
		{
			return false;
		}

		if (aData.isEmpty())
		{
			if (data.startsWith(ASCII::NAK))
			{
				aData = data.left(1);

				return true;
			}
			else if (data.startsWith(ASCII::ACK))
			{
				data.remove(0, 1);
			}
		}

		aData.append(data);

		if (!length && (aData.size() > 2))
		{
			length = aData[1] + 3;
		}

		if (length && (aData.size() >= length))
		{
			return true;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool ShtrihFRProtocol::sendACK()
{
	if (!mPort->write(QByteArray(1, ASCII::ACK)))
	{
		toLog(LogLevel::Error, "Shtrih: Failed to send ACK packet");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool ShtrihFRProtocol::sendNAK()
{
	if (!mPort->write(QByteArray(1, ASCII::NAK)))
	{
		toLog(LogLevel::Error, "Shtrih: Failed to send NAK");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool ShtrihFRProtocol::sendENQ()
{
	if (!mPort->write(QByteArray(1, ASCII::ENQ)))
	{
		toLog(LogLevel::Error, "Shtrih: Failed to send ENQ");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
