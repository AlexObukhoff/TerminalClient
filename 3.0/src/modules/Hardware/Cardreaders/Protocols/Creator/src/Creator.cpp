/* @file Протокол Creator. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/HardwareConstants.h"

// Project
#include "Creator.h"
#include "CreatorConstants.h"

//--------------------------------------------------------------------------------
Creator::Creator() : mIOLogsDebugMode(false)
{
}

//-------------------------------------------------------------------------------
bool Creator::sendPacket(const QByteArray & aData)
{
	int packetAmount = qCeil(double(aData.size()) / CCreator::USBDataSize);

	for (int i = 0; i < packetAmount; ++i)
	{
		SleepHelper::msleep(1);

		QByteArray packet = aData.mid(i * CCreator::USBDataSize, CCreator::USBDataSize);
		packet += QByteArray(CCreator::USBDataSize - packet.size(), ASCII::NUL);
		packet.prepend(ASCII::NUL);

		QString loggedRequest = packet.toHex();
		loggedRequest.replace(QRegExp("(00)+$"), CCreator::NULLogPostfix);

		toLog(LogLevel::Debug, "USB >> " + loggedRequest);

		if (!mPort->write(packet))
		{
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool Creator::checkUSBData(QByteArray & aData, int & aBase) const
{
	while (aData.size() > (aBase + 1))
	{
		char prefix = aData[aBase + 1];

		if ((prefix == ASCII::ACK) || (prefix == ASCII::NAK))
		{
			aBase += CCreator::USBPacketSize;
		}
		else if (prefix == CCreator::Prefix)
		{
			if (aData.size() >= (aBase + 4))
			{
				bool OK;
				int length = aData.mid(aBase + 2, 2).toHex().toUShort(&OK, 16);

				if (!OK || !length)
				{
					return false;
				}

				int oldBase = aBase;
				int packetAmount = qCeil(double(length + 5) / CCreator::USBDataSize);
				aBase += packetAmount * CCreator::USBPacketSize;

				int postfixIndex = oldBase + length + packetAmount + 3;
				int newAnswerSize = oldBase + CCreator::USBPacketSize;

				if (aData[postfixIndex] == ASCII::NUL)
				{
					for (int i = 0; i < packetAmount; ++i)
					{
						int basePacketIndex = (packetAmount - i - 1) * CCreator::USBPacketSize;
						
						if (aData.mid(basePacketIndex, CCreator::USBPacketSize) == CCreator::EmptyUSBPacket)
						{
							newAnswerSize = oldBase + basePacketIndex;
						}
					}
				}

				aData = aData.left(newAnswerSize);
			}
		}
		else
		{
			return prefix == ASCII::NUL;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
void Creator::trimUSBData(QByteArray & aData)
{
	for (int i = 0; i < aData.size(); i += CCreator::USBDataSize)
	{
		aData.remove(i, 1);
	}

	int j = 0;

	while (j < aData.size())
	{
		if ((aData[j] == ASCII::ACK) || (aData[j] == ASCII::NAK))
		{
			aData.remove(j + 1, CCreator::USBDataSize - 1);
			j++;
		}
		else if (aData[j] == CCreator::Prefix)
		{
			int length = aData.mid(j + 1, 2).toHex().toUShort(0, 16);
			j += length + 5;
		}
		else
		{
			aData.chop(aData.size() - j);
		}
	}
}

//--------------------------------------------------------------------------------
bool Creator::receivePacket(QByteArray & aData)
{
	QByteArray answer;

	QTime clockTimer;
	clockTimer.restart();

	int base = 0;
	bool calcBaseOK = true;
	QStringList logBuffer;

	do
	{
		if (!mPort->read(answer, 1))
		{
			return false;
		}

		int size = answer.size();

		if ((size < CCreator::USBAnswerSize) && !(size % CCreator::USBPacketSize))
		{
			for (int i = 0; i < size; i += CCreator::USBPacketSize)
			{
				logBuffer << answer.mid(i, CCreator::USBPacketSize).toHex();
			}
		}
		else
		{
			QString loggedAnswer = answer.toHex();
			loggedAnswer.replace(QRegExp("(00)+$"), CCreator::NULLogPostfix);
			logBuffer << loggedAnswer;
		}

		int index = -1;
		while ((++index < answer.size()) && !answer[index]) {}
		index = qFloor(index / CCreator::USBPacketSize) * CCreator::USBPacketSize;
		answer = answer.mid(index);

		aData.append(answer);

		calcBaseOK = checkUSBData(aData, base);
	}
	while (calcBaseOK && (!base || (aData.size() < base)) && (clockTimer.elapsed() < CCreator::Timeouts::SeriesOfPackets));

	int NULAnswerIndex = 0;
	int i = -1;
	auto logNULCount = [&] () { if (NULAnswerIndex && i) logBuffer[i - 1] = QString::number(NULAnswerIndex) + logBuffer[i - 1]; };

	while (++i < logBuffer.size())
	{
		if (logBuffer[i] != CCreator::NULLogPostfix)
		{
			logNULCount();
			NULAnswerIndex = 0;
		}
		else if (NULAnswerIndex++)
		{
			logBuffer.removeAt(i--);
		}
	}

	logNULCount();
	logBuffer.removeAll("");

	toLog(LogLevel::Debug, "USB << \n" + logBuffer.join("\n"));

	trimUSBData(aData);

	return true;
}

//--------------------------------------------------------------------------------
char Creator::calcCRC(const QByteArray & aData)
{
	if (aData.isEmpty())
	{
		return 0;
	}

	char CRC = aData[0];

	for (int i = 1; i < aData.size(); ++i)
	{
		CRC ^= aData[i];
	}

	return CRC;
}

//--------------------------------------------------------------------------------
void Creator::packData(const QByteArray & aCommandPacket, QByteArray & aPacket)
{
	aPacket.append(CCreator::Prefix);

	int length = aCommandPacket.size();
	aPacket.append(char(length >> 8));
	aPacket.append(char(length));

	aPacket.append(aCommandPacket);
	aPacket.append(CCreator::Postfix);
	aPacket.append(calcCRC(aPacket));
}

//--------------------------------------------------------------------------------
TResult Creator::checkAnswer(const QByteArray & aAnswer)
{
	// мин. длина ответа
	if (aAnswer.size() < CCreator::MinPacketAnswerSize)
	{
		toLog(LogLevel::Error, QString("Creator: total answer length < %1").arg(CCreator::MinPacketAnswerSize));
		return aAnswer.isEmpty() ? CommandResult::NoAnswer : CommandResult::Protocol;
	}

	// длина
	int length = aAnswer.size() - 5;
	int dataLength = aAnswer.mid(1, 2).toHex().toUShort(0, 16);

	if (length != dataLength)
	{
		toLog(LogLevel::Error, QString("Creator: length = %1, need = %2").arg(length).arg(dataLength));
		return CommandResult::Protocol;
	}

	// CRC
	char CRC = calcCRC(aAnswer.left(aAnswer.size() - 1));
	char dataCRC = aAnswer[aAnswer.size() - 1];

	if (CRC != dataCRC)
	{
		toLog(LogLevel::Error, QString("Creator: CRC = %1, need = 0x%2").arg(ProtocolUtils::toHexLog(CRC)).arg(ProtocolUtils::toHexLog(dataCRC)));
		return CommandResult::Protocol;
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
TResult Creator::processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, bool aIOLogsDebugMode)
{
	mIOLogsDebugMode = aIOLogsDebugMode;

	QByteArray packet;
	packData(aCommandData, packet);

	QByteArray answer;
	int commandNAKIndex = 0;

	do
	{
		answer.clear();		

		if (commandNAKIndex)
		{
			toLog(LogLevel::Warning, "Creator: Failed to send the command, trying to repeat after NAK, attempt " + QString::number(commandNAKIndex + 1));
			SleepHelper::msleep(CCreator::NAKAnswerPause);
		}

		toLog(LogLevel::Normal, QString("Creator: >> {%1}").arg(packet.toHex().data()));

		if (!sendPacket(packet))
		{
			return CommandResult::Port;
		}

		TResult result = receiveAnswer(answer);

		if (!result)
		{
			return result;
		}

		if (!answer.startsWith(ASCII::NAK))
		{
			aUnpackedAnswer = answer.mid(3, answer.size() - 5);

			return CommandResult::OK;
		}
	}
	while (++commandNAKIndex < CCreator::MaxRepeatCommandNAK);

	toLog(LogLevel::Error, QString("Creator: Maximum quantity attempts of sending the command = %1 is exceeded.").arg(CCreator::MaxRepeatCommandNAK));

	return CommandResult::Transport;
}

//--------------------------------------------------------------------------------
TResult Creator::receiveAnswer(QByteArray & aAnswer)
{
	int answerNAKIndex = 0;
	TResult result = CommandResult::OK;

	do
	{
		if (answerNAKIndex)
		{
			toLog(LogLevel::Warning, "Creator: Failed to check the answer, trying to repeat using NAK, attempt " + QString::number(answerNAKIndex + 1));

			if (!sendNAK())
			{
				return CommandResult::Port;
			}
		}

		if (!readAnswer(aAnswer))
		{
			return CommandResult::Port;
		}
		else if (aAnswer.isEmpty())
		{
			return CommandResult::NoAnswer;
		}
		else if (aAnswer.startsWith(ASCII::NAK))
		{
			return CommandResult::OK;
		}

		result = checkAnswer(aAnswer);

		if (result)
		{
			return sendACK() ? CommandResult::OK : CommandResult::Port;
		}
	}
	while (++answerNAKIndex < CCreator::MaxRepeatCommandNAK);

	toLog(LogLevel::Error, QString("Creator: Maximum quantity attempts of reading the answer = %1 is exceeded.").arg(CCreator::MaxRepeatCommandNAK));

	return result;
}

//--------------------------------------------------------------------------------
bool Creator::readAnswer(QByteArray & aAnswer)
{
	QByteArray answer;
	ushort length = 0;
	bool ACK = false;

	QTime clockTimer;
	clockTimer.restart();

	do
	{
		answer.clear();

		if (!receivePacket(answer))
		{
			return false;
		}

		aAnswer.append(answer);

		if (!ACK && aAnswer.startsWith(ASCII::ACK))
		{
			aAnswer.remove(0, 1);
			ACK = true;
		}
		else if (aAnswer.startsWith(ASCII::NAK))
		{
			toLog(LogLevel::Warning, "Creator: Answer contains NAK");
			return true;
		}

		if (aAnswer.size() > 2)
		{
			length = aAnswer.mid(1, 2).toHex().toUShort(0, 16);
		}
	}
	while ((clockTimer.elapsed() < CCreator::Timeouts::DefaultAnswer) && ((aAnswer.size() < length) || !length));

	if (!aAnswer.isEmpty())
	{
		LogLevel::Enum logLevel = mIOLogsDebugMode ? LogLevel::Debug : LogLevel::Normal;
		toLog(logLevel, QString("Creator: << {%1}").arg(aAnswer.toHex().data()));

		if (mIOLogsDebugMode)
		{
			toLog(LogLevel::Normal, "Creator: << SMTH");
		}
	}
	else
	{
		toLog(LogLevel::Normal, "Creator: << {}");
	}

	return true;
}

//--------------------------------------------------------------------------------
bool Creator::sendACK()
{
	if (!sendPacket(QByteArray(1, ASCII::ACK)))
	{
		toLog(LogLevel::Error, "Creator: Failed to send ACK");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool Creator::sendNAK()
{
	if (!sendPacket(QByteArray(1, ASCII::NAK)))
	{
		toLog(LogLevel::Error, "Creator: Failed to send NAK");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
void Creator::setPort(SDK::Driver::IIOPort * aPort)
{
	ProtocolBase::setPort(aPort);

	if (mPort)
	{
		mPort->setLog(getLog());

		QVariantMap configuration;
		configuration.insert(CHardware::Port::MaxReadingSize, CCreator::USBAnswerSize);
		mPort->setDeviceConfiguration(configuration);
	}
}

//--------------------------------------------------------------------------------
