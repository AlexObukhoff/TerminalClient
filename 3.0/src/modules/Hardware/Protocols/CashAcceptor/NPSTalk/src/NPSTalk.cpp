/* @file Протокол NPSTalk. */

// STL
#include <algorithm>

// Project
#include "NPSTalk.h"

//--------------------------------------------------------------------------------
void NPSTalkProtocol::setAddress(uchar aAddress)
{
	mDeviceAddress = aAddress;
}

//--------------------------------------------------------------------------------
bool NPSTalkProtocol::check(const QByteArray & aCommandData, const QByteArray & aAnswerData)
{
	using namespace ProtocolUtils;

	if (aAnswerData.size() < NPSTalk::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("Too few bytes in answer = %1, need min = %2").arg(aAnswerData.size()).arg(NPSTalk::MinAnswerSize));
		return false;
	}

	if (aAnswerData[0] != NPSTalk::Prefix)
	{
		toLog(LogLevel::Error, QString("Wrong prefix = %1, need = %2").arg(toHexLog(aAnswerData[0])).arg(toHexLog(NPSTalk::Prefix)));
		return false;
	}

	char postfix = aAnswerData[aAnswerData.size() - 2];

	if (postfix != NPSTalk::Postfix)
	{
		toLog(LogLevel::Error, QString("Wrong postfix = %1, need = %2").arg(toHexLog(postfix)).arg(toHexLog(NPSTalk::Postfix)));
		return false;
	}

	if (aCommandData[0] != aAnswerData[0])
	{
		toLog(LogLevel::Error, QString("Wrong command in answer = %1, need = %2").arg(toHexLog(aCommandData[0])).arg(toHexLog(aAnswerData[0])));
		return false;
	}

	char dataCRC = 0;
	std::for_each(aAnswerData.begin() + 1, aAnswerData.end() - 1, [&dataCRC] (char element) {dataCRC ^= element;} );
	char answerCRC = aAnswerData[aAnswerData.size() - 1];

	if (dataCRC != answerCRC)
	{
		toLog(LogLevel::Error, QString("Wrong CRC = %1, need = %2").arg(toHexLog(answerCRC)).arg(toHexLog(dataCRC)));
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------------
TResult NPSTalkProtocol::processCommand(const QByteArray & aCommanddata, QByteArray & aAnswerData)
{
	QByteArray request;
	request.append(NPSTalk::Prefix);
	request.append(aCommanddata);
	request.append(NPSTalk::Postfix);

	char result = 0;
	std::for_each(request.begin() + 1, request.end(), [&result] (char element) {result ^= element;} );
	request.append(result);

	bool error  = false;
	int repeat = 0;

	do
	{
		toLog(LogLevel::Normal, QString("NPSTalk: >> {%1}").arg(request.toHex().data()));

		if (!mPort->write(request) || !getAnswer(aAnswerData))
		{
			return CommandResult::Port;
		}

		if (aAnswerData.isEmpty())
		{
			return CommandResult::NoAnswer;
		}

		error = !check(request, aAnswerData);
	}
	while(error && (++repeat < NPSTalk::MaxBadAnswerRepeats));

	if (error)
	{
		return CommandResult::NoAnswer;
	}

	aAnswerData = aAnswerData.mid(2, aAnswerData.size() - 4);

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool NPSTalkProtocol::getAnswer(QByteArray & aAnswer)
{
	aAnswer.clear();

	QTime timer;
	timer.restart();

	do
	{
		QByteArray answer;

		if (!mPort->read(answer))
		{
			return false;
		}

		aAnswer.append(answer);
	}
	while ((timer.elapsed() < NPSTalk::Timeout) && ((aAnswer.size() < 4) || (aAnswer[aAnswer.size() - 2] != NPSTalk::Postfix)));

	toLog(LogLevel::Normal, QString("NPSTalk: << {%1}").arg(aAnswer.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
