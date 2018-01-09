/* @file Протокол сторожевого таймера OSMP 2.5. */

// STL
#include <numeric>

// Project
#include "OSMPWD.h"
#include "OSMPWDConstants.h"

//--------------------------------------------------------------------------------
const uchar OSMPWDProtocol::calcCRC(const QByteArray & aData)
{
	int sum = std::accumulate(aData.begin(), aData.end(), 0, [] (uchar arg1, uchar arg2) -> uchar { return arg1 + uchar(arg2); });

	return uchar(256 - sum);
}

//--------------------------------------------------------------------------------
bool OSMPWDProtocol::checkAnswer(const QByteArray & aAnswerData)
{
	// размер
	if (aAnswerData.size() < COSMPWD::MinPacketAnswerSize)
	{
		toLog(LogLevel::Error, QString("OSMPWD: The length of the packet is less than %1 byte").arg(COSMPWD::MinPacketAnswerSize));
		return false;
	}

	// префикс
	char prefix = aAnswerData[0];

	if (prefix != COSMPWD::Prefix)
	{
		toLog(LogLevel::Error, QString("OSMPWD: Invalid prefix = %1, need = %2")
			.arg(ProtocolUtils::toHexLog(prefix))
			.arg(ProtocolUtils::toHexLog(COSMPWD::Prefix)));
		return false;
	}

	// длина
	int answerLength = aAnswerData.mid(3, aAnswerData.size() - 4).size();
	int length = aAnswerData[2];

	if (length != answerLength)
	{
		toLog(LogLevel::Error, QString("OSMPWD: Invalid length = %1, need = %2").arg(length).arg(answerLength));
		return false;
	}

	// CRC
	int index = aAnswerData.size() - 1;
	char answerCRC = calcCRC(aAnswerData.left(index));
	char CRC = aAnswerData[index];

	if (CRC != answerCRC)
	{
		toLog(LogLevel::Error, QString("OSMPWD: Invalid CRC = %1, need %2")
			.arg(ProtocolUtils::toHexLog(CRC))
			.arg(ProtocolUtils::toHexLog(answerCRC)));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
TResult OSMPWDProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedData, bool aNeedAnswer)
{
	QByteArray packet;
	packet.append(COSMPWD::Prefix);
	packet.append(aCommandData[0]);
	uchar length = uchar(aCommandData.size() - 1);
	packet.append(length);
	packet.append(aCommandData.mid(1));
	packet.append(calcCRC(packet));

	// Выполняем команду
	toLog(LogLevel::Normal, QString("OSMPWD: >> {%1}").arg(packet.toHex().data()));
	QByteArray answerData;

	if (!mPort->write(packet) || !read(answerData))
	{
		return CommandResult::Port;
	}

	toLog(LogLevel::Normal, QString("OSMPWD: << {%1}").arg(answerData.toHex().data()));

	if (answerData.isEmpty())
	{
		return CommandResult::NoAnswer;
	}

	// распаковываем
	if (!checkAnswer(answerData))
	{
		toLog(LogLevel::Error, "OSMPWD: Failed to unpack data");
		return CommandResult::Protocol;
	}

	if (aNeedAnswer)
	{
		aUnpackedData = answerData.mid(3, answerData.size() - 4);
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool OSMPWDProtocol::read(QByteArray & aData)
{
	aData.clear();

	QTime clockTimer;
	clockTimer.start();

	do
	{
		QByteArray answerData;

		if (!mPort->read(answerData))
		{
			return false;
		}

		aData.append(answerData);

		if ((aData.size() >= COSMPWD::MinPacketAnswerSize) && (aData.mid(3, aData.size() - 4).size() >= int(answerData[2])))
		{
			break;
		}
	}
	while (clockTimer.elapsed() < COSMPWD::DefaultTimeout);

	return true;
}

//--------------------------------------------------------------------------------
