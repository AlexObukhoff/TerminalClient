/* @file Протокол AFP. */

// STL
#include <numeric>

// Modules
#include "Hardware/Common/HardwareConstants.h"
#include "AFPFRConstants.h"

// Project
#include "AFPFR.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
AFPFRProtocol::AFPFRProtocol(): mId(ASCII::Space)
{
}

//--------------------------------------------------------------------------------
char AFPFRProtocol::calcCRC(const QByteArray & aData)
{
	char sum = aData[0];

	for (int i = 1; i < aData.size(); ++i)
	{
		sum ^= aData[i];
	}

	return sum;
}

//--------------------------------------------------------------------------------
bool AFPFRProtocol::check(const QByteArray & aRequest, const QByteArray & aAnswer)
{
	int size = aAnswer.size();

	if (size < CAFPFR::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("AFP: Too few bytes in answer = %1, need min = %2").arg(size).arg(CAFPFR::MinAnswerSize));
		return false;
	}

	if (aAnswer[0] != CAFPFR::Prefix)
	{
		toLog(LogLevel::Error, QString("AFP: Invalid prefix = %1, need = %2")
			.arg(ProtocolUtils::toHexLog(aAnswer[0]))
			.arg(ProtocolUtils::toHexLog(CAFPFR::Prefix)));
		return false;
	}

	if (aAnswer[1] != char(mId))
	{
		toLog(LogLevel::Error, QString("AFP: Invalid Id = %1, need = %2")
			.arg(ProtocolUtils::toHexLog(aAnswer[1]))
			.arg(ProtocolUtils::toHexLog(mId)));
		return false;
	}

	QString requestCommand = aRequest.mid(6, 2);
	QString  answerCommand = aAnswer.mid(2, 2);

	if (requestCommand != answerCommand)
	{
		toLog(LogLevel::Error, QString("AFP: Invalid command in answer = 0x%1, need = 0x%2")
			.arg(answerCommand.toUpper())
			.arg(requestCommand.toUpper()));
		return false;
	}

	char postfix = aAnswer[size - 3];

	if (postfix != CAFPFR::Postfix)
	{
		toLog(LogLevel::Error, QString("AFP: Invalid postfix = %1, need = %2")
			.arg(ProtocolUtils::toHexLog(postfix))
			.arg(ProtocolUtils::toHexLog(CAFPFR::Postfix)));
		return false;
	}

	bool OK;
	QByteArray answerCRCData = aAnswer.right(2);
	char answerCRC = char(answerCRCData.toUShort(&OK, 16));
	char dataCRC = calcCRC(aAnswer.mid(1, size - 3));

	if (!OK)
	{
		toLog(LogLevel::Error, QString("AFP: Failed to parse answer CRC = 0x%1").arg(answerCRCData.toHex().data()));
		return false;
	}

	if (dataCRC != answerCRC)
	{
		toLog(LogLevel::Error, QString("AFP: Wrong CRC = %1, need = %2")
			.arg(ProtocolUtils::toHexLog(answerCRC))
			.arg(ProtocolUtils::toHexLog(dataCRC)));
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------------
TResult AFPFRProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aAnswer, int aTimeout)
{
	QByteArray request;

	request.append(CAFPFR::Prefix);
	request.append(CAFPFR::Password);

	mId = (mId == CAFPFR::MaxId) ? ASCII::Space : ++mId;

	request.append(mId);
	request.append(aCommandData);
	request.append(CAFPFR::Separator);
	request.append(CAFPFR::Postfix);

	char CRC = calcCRC(request.mid(1));
	QByteArray CRCData = QByteArray::number(CRC, 16).rightJustified(2, ASCII::Zero).right(2).toUpper();
	request.append(CRCData);

	toLog(LogLevel::Normal, QString("AFP: >> {%1}").arg(request.toHex().data()));
	aAnswer.clear();

	if (!mPort->write(request) || !getAnswer(aAnswer, aTimeout))
	{
		return CommandResult::Port;
	}

	toLog(LogLevel::Normal, QString("AFP: << {%1}").arg(aAnswer.toHex().data()));

	if (aAnswer.isEmpty())
	{
		return CommandResult::NoAnswer;
	}

	if (!check(request, aAnswer))
	{
		return CommandResult::Protocol;
	}

	aAnswer = aAnswer.mid(4, aAnswer.size() - 7);

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool AFPFRProtocol::getAnswer(QByteArray & aAnswer, int aTimeout)
{
	aAnswer.clear();

	QTime clockTimer;
	clockTimer.start();

	do
	{
		QByteArray data;

		if (!mPort->read(data, 20))
		{
			return false;
		}

		aAnswer.append(data);
		int size = aAnswer.size();

		if ((size > 2) && (aAnswer[size - 3] == CAFPFR::Postfix))
		{
			break;
		}
	}
	while (clockTimer.elapsed() < aTimeout);

	return true;
}

//--------------------------------------------------------------------------------
