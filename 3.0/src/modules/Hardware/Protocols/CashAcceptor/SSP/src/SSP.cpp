/* @file Протокол SSP. */

// STL
#include <cmath>

// Project
#include "SSP.h"
#include "SSPConstants.h"

using namespace SDK::Driver;

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
	uchar high = 0xFF;
	uchar low  = 0xFF;

	for (int i = 0; i < aData.size(); ++i)
	{
		uchar value = uchar(aData[i]) ^ high;
		ushort CRC = value << 8;

		for (int j = 0; j < 8; ++j) 
		{
			if (CRC & CSSP::LastBit) 
			{
				CRC = (CRC << 1) ^ CSSP::Polynominal;
			}
			else
			{
				CRC = CRC << 1;
			}
		}

		high = uchar(CRC >> 8) ^ low;
		low  = uchar(CRC);
	}

	return (ushort(high) << 8) + low;
}

//--------------------------------------------------------------------------------
bool SSPProtocol::check(const QByteArray & aAnswer)
{
	// минимальный размер ответа
	if (aAnswer.size() < CSSP::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("SSP: Invalid answer length = %1, need %2 minimum").arg(aAnswer.size()).arg(CSSP::MinAnswerSize));
		return false;
	}

	// первый байт
	char prefix = aAnswer[0];

	if (prefix != CSSP::Prefix[0])
	{
		toLog(LogLevel::Error, QString("SSP: Invalid prefix = %1, need = %2")
			.arg(ProtocolUtils::toHexLog(prefix))
			.arg(ProtocolUtils::toHexLog(CSSP::Prefix[0])));
		return false;
	}

	// адрес
	char address = aAnswer[1] & ~CSSP::SequenceFlag;

	if (address != mAddress)
	{
		toLog(LogLevel::Error, QString("SSP: Invalid address = %1, need = %2")
			.arg(ProtocolUtils::toHexLog(address))
			.arg(ProtocolUtils::toHexLog(mAddress)));
		return false;
	}

	// длина
	int answerLength = aAnswer.size() - 5;
	int length = uchar(aAnswer[2]);

	if (length != (aAnswer.size() - 5))
	{
		toLog(LogLevel::Error, QString("SSP: Invalid length = %1, need %2").arg(length).arg(answerLength));
		return false;
	}

	// CRC
	ushort answerCRC = calcCRC(aAnswer.mid(1, aAnswer.size() - 3));
	ushort CRC = qToBigEndian(aAnswer.right(2).toHex().toUShort(0, 16));

	if (CRC != answerCRC)
	{
		toLog(LogLevel::Error, QString("SSP: Invalid CRC = %1, need %2")
			.arg(ProtocolUtils::toHexLog(CRC))
			.arg(ProtocolUtils::toHexLog(answerCRC)));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
TResult SSPProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData, bool aSetSync)
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

		if (!mPort->write(request) || !getAnswer(answer))
		{
			return CommandResult::Port;
		}

		toLog(LogLevel::Normal, QString("SSP: << {%1}").arg(answer.toHex().data()));

		if (check(answer))
		{
			mSequenceFlag = !aSetSync && !mSequenceFlag;
			aAnswerData = answer.mid(3, answer.size() - 5);

			return CommandResult::OK;
		}
	}
	while(checkingCounter <= CSSP::MaxRepeatPacket);

	mSequenceFlag = !aSetSync && !mSequenceFlag;

	return CommandResult::Protocol;
}

//--------------------------------------------------------------------------------
bool SSPProtocol::getAnswer(QByteArray & aAnswer)
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
	while ((clockTimer.elapsed() < CSSP::AnswerTimeout) && ((aAnswer.size() < (length + 5)) || !length));

	return true;
}

//--------------------------------------------------------------------------------
