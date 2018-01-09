/* @file Протокол CCNet. */

// STL
#include <cmath>

// SDK
#include <SDK/Drivers/IOPort/COMParameters.h>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/ASCII.h"

// Project
#include "CCNet.h"
#include "CCNetConstants.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
CCNetProtocol::CCNetProtocol(): mAddress(0)
{
}

//--------------------------------------------------------------------------------
void CCNetProtocol::setAddress(char aAddress)
{
	mAddress = aAddress;
}

//--------------------------------------------------------------------------------
void CCNetProtocol::changePortParameters(TPortParameters aParameters)
{
	mPortParameters = aParameters;
}

//--------------------------------------------------------------------------------
ushort CCNetProtocol::calcCRC16(const QByteArray & aData)
{
	ushort CRC = 0;

	for (int i = 0; i < aData.size(); ++i)
	{
		ushort byteCRC = 0;
		ushort value = uchar(CRC ^ aData[i]);

		for (int j = 0; j < 8; ++j)
		{
			ushort data = byteCRC >> 1;
			byteCRC = ((byteCRC ^ value) & 1) ? (data ^ CCCNet::Polynominal) : data;
			value = value >> 1;
		}

		CRC = byteCRC ^ (CRC >> 8);
	}

	return CRC;
}

//--------------------------------------------------------------------------------
void CCNetProtocol::pack(QByteArray & aCommandData)
{
	aCommandData.prepend(mAddress);
	aCommandData.prepend(CCCNet::Prefix);

	int length = aCommandData.size() + 3;

	if (length < 256)
	{
		aCommandData.insert(2, char(length));
	}
	else
	{
		aCommandData.insert(2, ASCII::NUL);
		aCommandData.insert(5, QByteArray::fromHex(QString("%1").arg(length + 2, 4, 16, QChar(ASCII::Zero)).toLatin1()));
	}

	ushort CRC = calcCRC16(aCommandData);
	aCommandData.append(uchar(CRC));
	aCommandData.append(uchar(CRC >> 8));
}

//--------------------------------------------------------------------------------
QString CCNetProtocol::check(const QByteArray & aAnswer)
{
	// минимальный размер ответа
	if (aAnswer.size() < CCCNet::MinAnswerSize)
	{
		return QString("CCNet: Invalid answer length = %1, need %2 minimum").arg(aAnswer.size()).arg(CCCNet::MinAnswerSize);
	}

	// первый байт
	char prefix = aAnswer[0];

	if (prefix != CCCNet::Prefix)
	{
		return QString("CCNet: Invalid prefix = %1, need = %2")
			.arg(ProtocolUtils::toHexLog(prefix))
			.arg(ProtocolUtils::toHexLog(CCCNet::Prefix));
	}

	// адрес
	char address = aAnswer[1];

	if (address != mAddress)
	{
		return QString("CCNet: Invalid address = %1, need = %2")
			.arg(ProtocolUtils::toHexLog(address))
			.arg(ProtocolUtils::toHexLog(mAddress));
	}

	// длина
	int length = uchar(aAnswer[2]);

	if (length != aAnswer.size())
	{
		return QString("CCNet: Invalid length = %1, need %2").arg(aAnswer.size()).arg(length);
	}

	// CRC
	ushort answerCRC = calcCRC16(aAnswer.left(length - 2));
	ushort CRC = qToBigEndian(aAnswer.right(2).toHex().toUShort(0, 16));

	if (CRC != answerCRC)
	{
		return QString("CCNet: Invalid CRC = %1, need %2")
			.arg(ProtocolUtils::toHexLog(CRC))
			.arg(ProtocolUtils::toHexLog(answerCRC));
	}

	return "";
}

//--------------------------------------------------------------------------------
TResult CCNetProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData, const CCCNet::Commands::SData & aData)
{
	// Формируем пакет запроса
	QByteArray request = aCommandData;
	pack(request);

	// Выполняем команду
	int NAKCounter = 1;
	int checkingCounter = 1;

	do
	{
		toLog(LogLevel::Normal, QString("CCNet: >> {%1}").arg(QString(request.toHex())));
		aAnswerData.clear();

		if (!mPort->write(request))
		{
			return CommandResult::Port;
		}

		if (!mPortParameters.isEmpty())
		{
			SleepHelper::msleep(CCCNet::ChangingBaudRatePause);

			int baudrate = mPortParameters[IOPort::COM::EParameters::BaudRate];
			bool result = mPort->setParameters(mPortParameters);
			mPortParameters.clear();

			if (!result)
			{
				toLog(LogLevel::Error, "CCNet: Failed to set port parameters");
				return CommandResult::Port;
			}

			toLog(LogLevel::Normal, QString("CCNet: Baud rate has changed to %1.").arg(baudrate));
		}

		TResult result = getAnswer(aAnswerData, aData);

		if (result == CommandResult::Transport)
		{
			NAKCounter++;
		}
		else if (result == CommandResult::Protocol)
		{
			checkingCounter++;
		}
		else
		{
			return result;
		}
	}
	while((NAKCounter <= CCCNet::MaxRepeatPacket) && (checkingCounter <= CCCNet::MaxRepeatPacket));

	return (checkingCounter <= CCCNet::MaxRepeatPacket) ? CommandResult::Transport : CommandResult::Protocol;
}

//--------------------------------------------------------------------------------
TResult CCNetProtocol::getAnswer(QByteArray & aAnswerData, const CCCNet::Commands::SData & aData)
{
	TAnswers answers;

	if (!readAnswers(answers, aData.timeout))
	{
		return CommandResult::Port;
	}

	if (answers.isEmpty())
	{
		return CommandResult::NoAnswer;
	}

	int index = -1;
	QStringList logs;
	aAnswerData = answers[0];

	for (int i = 0; i < answers.size(); ++i)
	{
		logs << check(answers[i]);

		if (logs.last().isEmpty())
		{
			index = i;
		}
	}

	for (int i = 0; i < answers.size(); ++i)
	{
		QString log = QString("CCNet: << {%1}").arg(answers[i].toHex().data());

		if (!logs[i].isEmpty())
		{
			toLog(LogLevel::Normal, log);
			toLog(LogLevel::Error, logs[i]);
		}
		else if (i != index)
		{
			toLog(LogLevel::Normal, log + " - omitted");
		}
		else
		{
			toLog(LogLevel::Normal, log);
		}
	}

	if (index == -1)
	{
		toLog(LogLevel::Normal, "CCNet: Answer does not contains any logic data or it is incomplete answer");
		return CommandResult::Protocol;
	}

	int length = uchar(answers[index][2]);
	aAnswerData = answers[index].mid(3, length - 5);

	if (aAnswerData[0] == CCCNet::NAK)
	{
		toLog(LogLevel::Normal, "CCNet: Answer contains NAK, attemp to repeat command");
		return CommandResult::Transport;
	}

	if (aData.hostACK)
	{
		sendACK();
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool CCNetProtocol::readAnswers(TAnswers & aAnswers, int aTimeout)
{
	QByteArray answer;

	int length = 0;
	int index = 0;

	QTime clockTimer;
	clockTimer.restart();

	do
	{
		QByteArray answerData;

		if (!mPort->read(answerData, 20))
		{
			return false;
		}

		answer.append(answerData);
		int begin = index;
		int lastBegin;

		do
		{
			lastBegin = begin;
			begin = answer.indexOf(CCCNet::Prefix, begin);

			if (begin == -1)
			{
				break;
			}
			else if (answer.size() > 2)
			{
				if (begin < answer.size())
				{
					index = begin;
				}

				length = uchar(answer[begin + 2]);
				begin += length;

				if (begin < answer.size())
				{
					index = begin;
				}
			}
		}
		while (lastBegin != begin);
	}
	while ((clockTimer.elapsed() < aTimeout) && ((answer.mid(index).size() != length) || !length));

	if (answer.isEmpty())
	{
		toLog(LogLevel::Normal, "CCNet: << {}");
		return true;
	}

	int size = answer.size();
	int begin = answer.indexOf(CCCNet::Prefix);

	if (begin == -1)
	{
		begin = size;
	}

	if (begin)
	{
		aAnswers << answer.mid(0, begin);
	}

	do
	{
		int next = size;
		int shiftLength = -1;

		if (size >= (begin + 3))
		{
			shiftLength = uchar(answer[begin + 2]);
			next = begin + shiftLength;
		}

		aAnswers << answer.mid(begin, next - begin);

		int shift = (shiftLength <= 0) ? 1 : shiftLength;
		begin = answer.indexOf(CCCNet::Prefix, begin + shift);
	}
	while (begin != -1);

	return true;
}

//--------------------------------------------------------------------------------
bool CCNetProtocol::sendACK()
{
	QByteArray command(1, CCCNet::ACK);
	pack(command);
	toLog(LogLevel::Normal, QString("CCNet: >> {%1} - ACK").arg(command.toHex().data()));

	return mPort->write(command);
}

//--------------------------------------------------------------------------------
