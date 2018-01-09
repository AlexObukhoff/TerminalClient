/* @file Протокол ФР АТОЛ. */

// STL
#include <cmath>

// Project
#include "AtolFR.h"
#include "AtolFRConstants.h"

//--------------------------------------------------------------------------------
const ushort AtolFRProtocol::calcCRC(const QByteArray & aData)
{
	if (!aData.size())
	{
		return 0;
	}

	ushort sum = aData[0];

	for (int i = 1; i < aData.size(); ++i)
	{
		sum ^= static_cast<uchar>(aData[i]);
	}

	return sum;
}

//--------------------------------------------------------------------------------
void AtolFRProtocol::packData(const QByteArray & aCommandPacket, QByteArray & aPacket)
{
	QByteArray passwordPacket(2, ASCII::NUL);

	// пакет с данными
	QByteArray dataPacket;

	// маскировка некоторых символов
	for (auto it = aCommandPacket.begin(); it != aCommandPacket.end(); ++it)
	{
		if ((*it == CAtolFR::MaskByte) ||
			(*it == CAtolFR::Postfix))
		{
			dataPacket.push_back(CAtolFR::MaskByte);
		}

		dataPacket.push_back(*it);
	}

	aPacket.push_back(passwordPacket);
	aPacket.push_back(dataPacket);

	// последний байт - фиксированный
	aPacket.push_back(CAtolFR::Postfix);

	// подсчитываем контрольную сумму
	ushort CRC = calcCRC(aPacket);

	// записываем контрольную сумму
	aPacket.push_back(static_cast<uchar>(CRC));

	// Первый байт - фиксированный
	aPacket.insert(0, CAtolFR::Prefix);
}

//--------------------------------------------------------------------------------
bool AtolFRProtocol::unpackData(const QByteArray & aPacket, QByteArray & aData)
{
	if (aPacket.size() < CAtolFR::MinPacketAnswerSize)
	{
		toLog(LogLevel::Error, QString("ATOL: The length of the packet is less than %1 byte").arg(CAtolFR::MinPacketAnswerSize));
		return false;
	}

	// Pr_KKM_4_15.pdf 14 страница Примечание 1
	QByteArray finalPacket;

	bool findPrefix = false;

	for (int i = 0; i < aPacket.size() - 1; ++i)
	{
		// 1. ищем STX (все байты, неравные STX, игнорируем).
		if (aPacket[i] == CAtolFR::Prefix && !findPrefix)
		{
			findPrefix = true;
			continue;
		}
		else if (!findPrefix)
		{
			continue;
		}

		// 2. После STX все байты рассматривать как данные кадра.
		finalPacket.push_back(aPacket[i]);

		if ((aPacket[i + 1] == ASCII::DLE || aPacket[i + 1] == ASCII::ETX) && aPacket[i] == ASCII::DLE)
		{
			// маскируемый байт
			i++;
			finalPacket.push_back(aPacket[i]);

			continue;
		}

		// 3. Принимать кадр до получения ETX.
		// 4. Если полученный байт ETX маскированный символом DLE, то
		// рассматривать его как часть данных и продолжать прием – п.3.
		if (aPacket[i] == ASCII::ETX)
		{
			// 5. Принять 1 байт после немаскированного ETX – <CRC>.
			finalPacket.push_back(aPacket[i + 1]);
			break;
		}
	}

	if (!findPrefix)
	{
		toLog(LogLevel::Error, QString("ATOL: Failed to find prefix = %1").arg(ProtocolUtils::toHexLog(CAtolFR::Prefix)));
		return false;
	}

	if (finalPacket.size() < (CAtolFR::MinPacketAnswerSize - 1))
	{
		toLog(LogLevel::Error, QString("ATOL: The length of the final packet = %1, need %2 minimum").arg(finalPacket.size()).arg(CAtolFR::MinPacketAnswerSize - 1));
		return false;
	}

	char postfix = finalPacket[finalPacket.size() - 2];

	if (postfix != CAtolFR::Postfix)
	{
		toLog(LogLevel::Error, QString("ATOL: Invalid postfix = %1, need = %2")
			.arg(ProtocolUtils::toHexLog(postfix))
			.arg(ProtocolUtils::toHexLog(CAtolFR::Postfix)));
		return false;
	}

	// обрезаем crc
	QByteArray tempAnswer = finalPacket.left(finalPacket.size() - 1);

	// проверяем контрольную сумму
	ushort localCRC = calcCRC(tempAnswer);
	ushort CRC = finalPacket[finalPacket.size() - 1] & 0x00FF;

	if (localCRC != CRC)
	{
		toLog(LogLevel::Error, "ATOL: Invalid CRC of the answer message.");
		return false;
	}

	// удаляем экранирующие символы
	QByteArray clearAnswer;

	for (auto it = tempAnswer.begin(); it != tempAnswer.end(); ++it)
	{
		if (*it != CAtolFR::MaskByte)
		{
			clearAnswer.push_back(*it);
		}
		else
		{
			if (((it + 1) != tempAnswer.end()) && ((*(it + 1) == CAtolFR::MaskByte) || (*(it + 1) == CAtolFR::Postfix)))
			{
				clearAnswer.push_back(*(it + 1));
				++it;
			}
		}
	}

	// записываем данные
	aData = clearAnswer.left(clearAnswer.size() - 1);

	return true;
}

//--------------------------------------------------------------------------------
TResult AtolFRProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, int aTimeout)
{
	// Упаковываем все остальные байты
	QByteArray packet;
	packData(aCommandData, packet);

	// Выполняем команду
	toLog(LogLevel::Normal, QString("ATOL: >> {%1}").arg(packet.toHex().data()));

	if (!execCommand(packet))
	{
		return CommandResult::Transport;
	}

	// получаем ответ
	QByteArray answer;

	if (!openReadSession(aTimeout))
	{
		return CommandResult::Transport;
	}

	bool readResult = read(answer);

	if (!answer.isEmpty())
	{
		sendACK();
		closeReadSession();
	}

	if (!readResult)
	{
		return CommandResult::Transport;
	}

	toLog(LogLevel::Normal, QString("ATOL: << {%1}").arg(answer.toHex().data()));

	// распаковываем
	if (!unpackData(answer, aUnpackedAnswer))
	{
		toLog(LogLevel::Error, "ATOL: Failed to unpack data");
		return CommandResult::Protocol;
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool AtolFRProtocol::execCommand(QByteArray & aPacket)
{
	if (!openWriteSession())
	{
		return false;
	}

	bool emptyAnswer = true;

	for (int i = 0; i < CAtolFR::MaxRepeatPacket; ++i)
	{
		QByteArray answerData;

		if (!mPort->write(aPacket) || !mPort->read(answerData, CAtolFR::Timeouts::OpeningSession))
		{
			return false;
		}

		if (!answerData.isEmpty())
		{
			if (answerData[0] == ASCII::ACK)
			{
				break;
			}
			else if (answerData[0] == ASCII::NAK) toLog(LogLevel::Warning, "ATOL: Answer contains NAK message");
			else if (answerData[0] == ASCII::ENQ) toLog(LogLevel::Warning, "ATOL: Answer contains ENQ message");
			else if (answerData[0] == ASCII::EOT) toLog(LogLevel::Warning, "ATOL: Answer contains EOT message");
			else
			{
				toLog(LogLevel::Warning, "ATOL: Answer contains Unknown type of the message");
			}

			if (emptyAnswer && (i == (CAtolFR::MaxRepeatPacket - 1)) && (answerData == QByteArray(answerData.size(), answerData[0])))
			{
				toLog(LogLevel::Warning, "ATOL: Trying again after not empty answers");
				i--;
			}

			emptyAnswer = false;
		}
	}

	return closeWriteSession();
}

//--------------------------------------------------------------------------------
bool AtolFRProtocol::read(QByteArray & aData)
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

		if (!answerData.isEmpty())
		{
			aData.append(answerData);

			//если получили постфикс и перед ним не стоит маска, значит это постфикс и надо выходить
			if ((aData.size() > 2) &&
			    (aData[aData.length() - 2] == CAtolFR::Postfix) &&
			    (aData[aData.length() - 3] != CAtolFR::MaskByte))
			{
				break;
			}
		}
	}
	while (clockTimer.elapsed() < CAtolFR::Timeouts::Default);

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRProtocol::closeWriteSession()
{
	if (!mPort->write(QByteArray(1, ASCII::EOT)))
	{
		toLog(LogLevel::Error, "ATOL: Failed to close write session");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRProtocol::closeReadSession()
{
	QByteArray answerData;

	if (!mPort->read(answerData, CAtolFR::Timeouts::Default) || !answerData.startsWith(ASCII::EOT))
	{
		toLog(LogLevel::Error, "ATOL: Failed to close read session");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRProtocol::sendACK()
{
	if (!mPort->write(QByteArray(1, ASCII::ACK)))
	{
		toLog(LogLevel::Error, "ATOL: Failed to send ACK");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AtolFRProtocol::openWriteSession()
{
	for (int i = 0; i < CAtolFR::MaxServiceRequests; ++i)
	{
		QByteArray answerData;

		if (!mPort->write(QByteArray(1, ASCII::ENQ)) || !mPort->read(answerData, CAtolFR::Timeouts::OpeningSession))
		{
			break;
		}

		if (answerData.startsWith(ASCII::ACK))
		{
			return true;
		}
		else if (answerData.startsWith(ASCII::NAK))
		{
			toLog(LogLevel::Warning, "ATOL: Answer contains NAK");

			SleepHelper::msleep(CAtolFR::Timeouts::NAKOpeningSession);
		}
	}

	toLog(LogLevel::Error, "ATOL: Failed to open write session");

	closeWriteSession();

	return false;
}

//--------------------------------------------------------------------------------
bool AtolFRProtocol::openReadSession(int aTimeout)
{
	QByteArray answerData;

	if (!mPort->read(answerData, aTimeout) || !answerData.startsWith(ASCII::ENQ) || !sendACK())
	{
		toLog(LogLevel::Error, "ATOL: Failed to open read session");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
