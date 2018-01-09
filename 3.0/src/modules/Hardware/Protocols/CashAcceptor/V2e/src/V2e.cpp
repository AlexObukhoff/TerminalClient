/* @file Протокол V2e. */

// STL
#include <numeric>

// Proejct
#include "V2e.h"
#include "V2eConstants.h"

//--------------------------------------------------------------------------------
ushort V2eProtocol::calcCRC(const QByteArray & aData)
{
	int sum = std::accumulate(aData.begin(), aData.end(), 0, [&] (ushort arg1, char arg2) -> ushort { return arg1 + uchar(arg2); });

	return ~ushort(sum) + 1;
}

//--------------------------------------------------------------------------------
void V2eProtocol::pack(QByteArray & aCommandData)
{
	int length = aCommandData.size() + 6;
	aCommandData.prepend(char(length));
	aCommandData.prepend(char(length >> 8));

	aCommandData.prepend(CV2e::ProtocolMark);
	aCommandData.prepend(CV2e::Prefix);

	ushort CRC = calcCRC(aCommandData);
	aCommandData.append(char(CRC >> 8));
	aCommandData.append(char(CRC));
}

//--------------------------------------------------------------------------------
bool V2eProtocol::check(const QByteArray & aAnswer, const QByteArray & aRequest)
{
	using namespace ProtocolUtils;

	// минимальная длина
	if (aAnswer.size() < CV2e::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("V2.2E: Invalid answer length = %1, need = %2 minimum").arg(aAnswer.size()).arg(CV2e::MinAnswerSize));
		return false;
	}

	// первый байт
	if (aAnswer[0] != CV2e::Prefix)
	{
		toLog(LogLevel::Error, QString("V2.2E: Invalid prefix: %1, need %2").arg(toHexLog<char>(aAnswer[0])).arg(toHexLog(CV2e::Prefix)));
		return false;
	}

	// длина
	int length = aAnswer.mid(2, 2).toHex().toUShort(0, 16);

	if (length != aAnswer.size())
	{
		toLog(LogLevel::Error, QString("V2.2E: Invalid length of the message: %1, need %2").arg(aAnswer.size()).arg(length));
		return false;
	}

	// команда
	if ((aAnswer[4] != CV2e::ACK) && (aAnswer[4] != CV2e::NAK) && (aAnswer[4] != CV2e::IRQ) &&
		(aRequest[4] != CV2e::Retransmit) && (aRequest[4] != aAnswer[4]))
	{
		toLog(LogLevel::Error, QString("V2.2E: Invalid command in answer: %1, need %2").arg(toHexLog<char>(aAnswer[4])).arg(toHexLog<char>(aRequest[4])));
		return false;
	}

	// CRC
	ushort CRC = calcCRC(aAnswer.left(length - 2));
	ushort answerCRC = aAnswer.right(2).toHex().toUShort(0, 16);

	if (CRC != answerCRC)
	{
		toLog(LogLevel::Error, QString("V2.2E: Invalid CRC of the message: %1, need %2").arg(CRC).arg(answerCRC));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
TResult V2eProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData)
{
	QByteArray request = aCommandData;
	pack(request);

	int repeat = 0;
	bool correct = true;

	do
	{
		if (!correct)
		{
			toLog(LogLevel::Normal, QString("V2.2E: iteration #%1 due to NAK in the answer").arg(repeat + 1));
		}

		TResult result = handleError(request, aAnswerData);

		if (!result)
		{
			return result;
		}

		correct = (aAnswerData.size() != 1) || (aAnswerData[0] != CV2e::NAK);
	}
	while(!correct && (++repeat < CV2e::MaxRepeat));

	if (!correct)
	{
		toLog(LogLevel::Error, "V2.2E: Failed to handle NAK in answer");
		return CommandResult::Transport;
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
TResult V2eProtocol::handleError(const QByteArray & aRequest, QByteArray & aAnswer)
{
	int repeat = 0;
	bool correct = true;
	QByteArray request(aRequest);

	do
	{
		if (!correct)
		{
			toLog(LogLevel::Normal, QString("V2.2E: iteration #%1 due to error in the answer").arg(repeat + 1));

			request = QByteArray(1, CV2e::Retransmit);
			pack(request);
		}

		toLog(LogLevel::Normal, QString("V2.2E: >> {%1}").arg(request.toHex().data()));

		if (!mPort->write(aRequest) || !getAnswer(aAnswer))
		{
			return CommandResult::Port;
		}

		if (aAnswer.isEmpty())
		{
			return CommandResult::NoAnswer;
		}

		correct = check(aAnswer, aRequest);

		aAnswer = aAnswer.mid(4, aAnswer.size() - 6);
	}
	while(!correct && (++repeat < CV2e::MaxRepeat));

	if (!correct)
	{
		toLog(LogLevel::Error, "V2.2E: Failed to handle errors in answer");
		return CommandResult::Transport;
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool V2eProtocol::getAnswer(QByteArray & aAnswer)
{
	aAnswer.clear();
	QByteArray data;
	ushort length = 0;

	QTime clockTimer;
	clockTimer.restart();

	do
	{
		data.clear();

		if (!mPort->read(data, 10))
		{
			return false;
		}

		aAnswer.append(data);

		if (aAnswer.size() > 3)
		{
			length = aAnswer.mid(2, 2).toHex().toUShort(0, 16);
		}
	}
	while ((clockTimer.elapsed() < CV2e::AnswerTimeout) && ((aAnswer.size() < length) || (!length)));

	toLog(LogLevel::Normal, QString("V2.2E: << {%1}").arg(aAnswer.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
