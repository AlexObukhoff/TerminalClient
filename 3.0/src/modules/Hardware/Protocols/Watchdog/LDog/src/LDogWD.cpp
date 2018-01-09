/* @file Протокол сторожевого таймера LDog. */

// STL
#include <numeric>

// Project
#include "LDogWD.h"
#include "LDogWDConstants.h"

//--------------------------------------------------------------------------------
uchar LDogWDProtocol::calcCRC(const QByteArray & aData)
{
	int sum = std::accumulate(aData.begin(), aData.end(), 0, [] (uchar arg1, uchar arg2) -> uchar { return arg1 + uchar(arg2); });

	return uchar(256 - sum);
}

//--------------------------------------------------------------------------------
bool LDogWDProtocol::checkAnswer(const QByteArray & aRequest, const QByteArray & aAnswer)
{
	// Минимальный размер ответа.
	if (aAnswer.size() < CLDogWD::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("LDog: Invalid answer length = %1, need %2 minimum").arg(aAnswer.size()).arg(CLDogWD::MinAnswerSize));
		return false;
	}

	// Наличие маски адреса.
	if (~aAnswer[0] & CLDogWD::AnswerMask)
	{
		toLog(LogLevel::Error, QString("LDog: No address mask = %1 in answer.").arg(ProtocolUtils::toHexLog(CLDogWD::AnswerMask)));
		return false;
	}

	// Соответствие адреса.
	char answerAddress = aAnswer[0] & ~CLDogWD::AnswerMask;
	char requestAddress = aRequest[0];

	if (answerAddress != requestAddress)
	{
		toLog(LogLevel::Error, QString("LDog: Invalid answer address = %1, need = %2.")
			.arg(ProtocolUtils::toHexLog(answerAddress))
			.arg(ProtocolUtils::toHexLog(requestAddress)));
		return false;
	}

	// Наличие маски команды.
	if (~aAnswer[1] & CLDogWD::AnswerMask)
	{
		toLog(LogLevel::Error, QString("LDog: No command mask = %1 in answer.").arg(ProtocolUtils::toHexLog(CLDogWD::AnswerMask)));
		return false;
	}

	// Соответствие команды.
	char answerCommand = aAnswer[1] & ~CLDogWD::AnswerMask;
	char requestCommand = aRequest[1];

	if (answerCommand != requestCommand)
	{
		toLog(LogLevel::Error, QString("LDog: Invalid command = %1 in answer, need = %2.")
			.arg(ProtocolUtils::toHexLog(answerCommand))
			.arg(ProtocolUtils::toHexLog(requestCommand)));
		return false;
	}

	// Последний байт.
	char postfix = aAnswer[aAnswer.size() - 1];

	if (postfix != CLDogWD::Postfix)
	{
		toLog(LogLevel::Error, QString("LDog: Invalid postfix = %1, need = %2.")
			.arg(ProtocolUtils::toHexLog(postfix))
			.arg(ProtocolUtils::toHexLog(CLDogWD::Postfix)));
		return false;
	}

	// Контрольная сумма.
	int answerSize = aAnswer.size() - 2;
	uchar CRC = aAnswer[answerSize];
	uchar realCRC = calcCRC(aAnswer.left(answerSize));

	if (CRC != realCRC)
	{
		toLog(LogLevel::Error, QString("LDog: Invalid CRC = %1, need = %2.")
			.arg(ProtocolUtils::toHexLog(CRC))
			.arg(ProtocolUtils::toHexLog(realCRC)));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
TResult LDogWDProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedData)
{
	QByteArray request;
	request.append(CLDogWD::Address);
	request.append(aCommandData);
	request.append(calcCRC(request));

	for (int i = 0; i < CLDogWD::ReplaceDataList.size(); ++i)
	{
		request.replace(CLDogWD::ReplaceDataList[i].first, CLDogWD::ReplaceDataList[i].second);
	}

	request.append(CLDogWD::Postfix);

	if (!mPort->write(request))
	{
		return CommandResult::Port;
	}

	QByteArray answer;

	if (!read(answer))
	{
		return CommandResult::Port;
	}

	if (answer.isEmpty())
	{
		return CommandResult::NoAnswer;
	}

	if (!checkAnswer(request, answer))
	{
		return CommandResult::Protocol;
	}

	answer = answer.mid(2, answer.size() - 4);

	for (int i = CLDogWD::ReplaceDataList.size(); i > 0; --i)
	{
		answer.replace(CLDogWD::ReplaceDataList[i - 1].second, QByteArray(1, CLDogWD::ReplaceDataList[i - 1].first));
	}

	aUnpackedData = answer;

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool LDogWDProtocol::read(QByteArray & aAnswer)
{
	QTime clockTimer;
	clockTimer.start();

	do
	{
		QByteArray answerData;

		if (!mPort->read(answerData))
		{
			return false;
		}

		aAnswer.append(answerData);
	}
	while ((clockTimer.elapsed() < CLDogWD::Timeouts::Answer) &&
		!((aAnswer.size() > 2) && (aAnswer.right(1)[0] == CLDogWD::Postfix) && (aAnswer.right(2) != CLDogWD::MaskedPostfix)));

	toLog(LogLevel::Normal, QString("LDog: << {%1}").arg(aAnswer.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
