/* @file Протокол ФР SPARK. */

// STL
#include <cmath>

// Project
#include "SparkFR.h"
#include "SparkFRConstants.h"

//--------------------------------------------------------------------------------
char SparkFRProtocol::calcCRC(const QByteArray & aData)
{
	char sum = aData[0];

	for (int i = 1; i < aData.size(); ++i)
	{
		sum ^= aData[i];
	}

	return sum;
}

//--------------------------------------------------------------------------------
bool SparkFRProtocol::unpack(QByteArray & aAnswerData)
{
	// размер
	if (aAnswerData.size() < CSparkFR::MinPacketAnswerSize)
	{
		toLog(LogLevel::Error, QString("SPARK: The length of the packet is less than %1 byte").arg(CSparkFR::MinPacketAnswerSize));
		return false;
	}

	// префикс
	char prefix = aAnswerData[0];

	if (prefix != CSparkFR::Prefix)
	{
		toLog(LogLevel::Error, QString("SPARK: Invalid prefix = %1, need = %2")
			.arg(ProtocolUtils::toHexLog(prefix))
			.arg(ProtocolUtils::toHexLog(CSparkFR::Prefix)));
		return false;
	}

	// постфикс
	int size = aAnswerData.size();
	char postfix = aAnswerData[size - 2];

	if (postfix != CSparkFR::Postfix)
	{
		toLog(LogLevel::Error, QString("SPARK: Invalid postfix = %1, need = %2")
			.arg(ProtocolUtils::toHexLog(postfix))
			.arg(ProtocolUtils::toHexLog(CSparkFR::Postfix)));
		return false;
	}

	// CRC
	char answerCRC = calcCRC(aAnswerData.mid(1, size - 2));
	char CRC = aAnswerData[size - 1];

	if (CRC != answerCRC)
	{
		toLog(LogLevel::Error, QString("SPARK: Invalid CRC = %1, need %2")
			.arg(ProtocolUtils::toHexLog(CRC))
			.arg(ProtocolUtils::toHexLog(answerCRC)));
		return false;
	}

	aAnswerData = aAnswerData.mid(1, size - 3);

	return true;
}

//--------------------------------------------------------------------------------
TResult SparkFRProtocol::performCommand(const QByteArray & aCommandData, QByteArray & aAnswerData, int aTimeout)
{
	int index = 1;

	do
	{
		aAnswerData.clear();
		QString log = QString("SPARK: >> {%1}").arg(aCommandData.toHex().data());

		if (index > 1)
		{
			log += ", iteration #" + QString::number(index);
		}

		toLog(LogLevel::Normal, log);

		if (!mPort->write(aCommandData))
		{
			return CommandResult::Port;
		}

		if (!readData(aAnswerData, aTimeout))
		{
			return CommandResult::Port;
		}
		else if (aAnswerData.isEmpty())
		{
			return CommandResult::NoAnswer;
		}
		else if (aAnswerData.startsWith(ASCII::ACK) || aAnswerData.startsWith(ASCII::ENQ))
		{
			return CommandResult::OK;
		}
		else if (!aAnswerData.startsWith(ASCII::NAK))
		{
			return unpack(aAnswerData) ? CommandResult::OK : CommandResult::Protocol;
		}

		toLog(LogLevel::Warning, "SPARK: Answer contains NAK");
	}
	while (aAnswerData.startsWith(ASCII::NAK) && (index++ < CSparkFR::MaxRepeatPacket));

	aAnswerData.clear();

	return CommandResult::Transport;
}

//--------------------------------------------------------------------------------
TResult SparkFRProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData, int aTimeout)
{
	QByteArray packet = aCommandData + CSparkFR::Postfix;
	packet.append(calcCRC(packet));
	packet.prepend(CSparkFR::Prefix);

	return performCommand(packet, aAnswerData, aTimeout);
}

//--------------------------------------------------------------------------------
TResult SparkFRProtocol::processCommand(char aCommand, QByteArray & aAnswerData, int aTimeout)
{
	return performCommand(QByteArray(1, aCommand), aAnswerData, aTimeout);
}

//--------------------------------------------------------------------------------
bool SparkFRProtocol::readData(QByteArray & aAnswerData, int aTimeout)
{
	aAnswerData.clear();

	QTime clockTimer;
	clockTimer.start();

	do
	{
		QByteArray data;

		if (!mPort->read(data, 50))
		{
			return false;
		}

		aAnswerData.append(data);

		int size = aAnswerData.size();

		if (aAnswerData.startsWith(ASCII::ENQ) ||
		    aAnswerData.startsWith(ASCII::ACK) ||
		    aAnswerData.startsWith(ASCII::NAK) ||
		   ((size > 1) && (aAnswerData[size - 2] == ASCII::ETX)))
		{
			break;
		}
	}
	while (clockTimer.elapsed() < aTimeout);

	toLog(LogLevel::Normal, QString("SPARK: << {%1}").arg(aAnswerData.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
