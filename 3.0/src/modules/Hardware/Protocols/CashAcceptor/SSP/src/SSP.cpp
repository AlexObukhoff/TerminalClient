/* @file Протокол SSP. */

// STL
#include <cmath>

// Project
#include "SSP.h"
#include "SSPConstants.h"

using namespace SDK::Driver;
using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
SSPProtocol::SSPProtocol() : mAddress(0), mSequenceFlag(false)
{
}

//--------------------------------------------------------------------------------
void SSPProtocol::setAddress(char aAddress)
{
	mAddress = aAddress;
}

//--------------------------------------------------------------------------------
ushort SSPProtocol::calcCRC(const QByteArray & aData)
{
	ushort CRC = 0xFFFF;

	for (int i = 0; i < aData.size(); ++i)
	{
		CRC ^= (aData[i] << 8);

		for (int j = 0; j < 8; ++j)
		{
			if (CRC & 0x8000)
			{
				CRC = (CRC << 1) ^ CSSP::Polynominal;
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
TResult SSPProtocol::check(const QByteArray & aAnswer)
{
	// минимальный размер ответа
	if (aAnswer.size() < CSSP::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("SSP: Too few bytes in answer = %1, need min = %2").arg(aAnswer.size()).arg(CSSP::MinAnswerSize));
		return CommandResult::Protocol;
	}

	// префикс
	char prefix = aAnswer[0];

	if (prefix != CSSP::Prefix[0])
	{
		toLog(LogLevel::Error, QString("SSP: Invalid prefix = %1, need = %2").arg(toHexLog(prefix)).arg(toHexLog(CSSP::Prefix[0])));
		return CommandResult::Protocol;
	}

	// адрес
	char address = aAnswer[1] & ~CSSP::SequenceFlag;

	if (address != mAddress)
	{
		toLog(LogLevel::Error, QString("SSP: Invalid address = %1, need = %2").arg(toHexLog(address)).arg(toHexLog(mAddress)));
		return CommandResult::Id;
	}

	// флаг последовательности
	bool sequenceFlag = aAnswer[1] & CSSP::SequenceFlag;

	if (sequenceFlag != mSequenceFlag)
	{
		toLog(LogLevel::Error, QString("SSP: Invalid sequence flag = %1, need = %2").arg(int(sequenceFlag)).arg(int(mSequenceFlag)));
		return CommandResult::Id;
	}

	// длина
	int answerLength = aAnswer.size() - 5;
	int length = uchar(aAnswer[2]);

	if (length != (aAnswer.size() - 5))
	{
		toLog(LogLevel::Error, QString("SSP: Invalid length = %1, need %2").arg(length).arg(answerLength));
		return CommandResult::Protocol;
	}

	// CRC
	ushort answerCRC = calcCRC(aAnswer.mid(1, aAnswer.size() - 3));
	ushort CRC = qToBigEndian(aAnswer.right(2).toHex().toUShort(0, 16));

	if (CRC != answerCRC)
	{
		toLog(LogLevel::Error, QString("SSP: Invalid CRC = %1, need %2").arg(toHexLog(CRC)).arg(toHexLog(answerCRC)));
		return CommandResult::CRC;
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
TResult SSPProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData, const CSSP::Commands::SData & aData)
{
	// Формируем пакет запроса
	char sequenceFlag = char(mSequenceFlag) * CSSP::SequenceFlag;

	QByteArray request;
	request.append(mAddress | sequenceFlag);
	request.append(uchar(aCommandData.size()));
	request.append(aCommandData);

	ushort CRC = calcCRC(request);
	request.append(uchar(CRC));
	request.append(uchar(CRC >> 8));

	request.replace(CSSP::Prefix, CSSP::MaskedPrefix);
	request.prepend(CSSP::Prefix);

	// Выполняем команду
	int checkingCounter = 1;

	do
	{
		QString log = QString("SSP: >> {%1}").arg(request.toHex().data());

		if (checkingCounter > 1)
		{
			log += ", iteration " + QString::number(checkingCounter);
		}

		toLog(LogLevel::Normal, log);
		QByteArray answer;

		if (!mPort->write(request) || !getAnswer(answer, aData.timeout))
		{
			return CommandResult::Port;
		}

		toLog(LogLevel::Normal, QString("SSP: << {%1}").arg(answer.toHex().data()));
		TResult result = check(answer);

		if (result)
		{
			mSequenceFlag = !aData.setSync && !mSequenceFlag;
			aAnswerData = answer.mid(3, answer.size() - 5);

			return CommandResult::OK;
		}
		else if (result == CommandResult::Protocol)
		{
			break;
		}
	}
	while (checkingCounter++ <= CSSP::MaxRepeatPacket);

	mSequenceFlag = !aData.setSync && !mSequenceFlag;

	return CommandResult::Protocol;
}

//--------------------------------------------------------------------------------
bool SSPProtocol::getAnswer(QByteArray & aAnswer, int aTimeout)
{
	uchar length = 0;

	QTime clockTimer;
	clockTimer.restart();

	do
	{
		QByteArray answer;

		if (!mPort->read(answer, 20))
		{
			return false;
		}

		aAnswer.append(answer);
		aAnswer.replace(CSSP::MaskedPrefix, CSSP::Prefix);

		if (aAnswer.size() > 2)
		{
			length = aAnswer[2];
		}
	}
	while ((clockTimer.elapsed() < aTimeout) && ((aAnswer.size() < (length + 5)) || !length));

	return true;
}

//--------------------------------------------------------------------------------
