/* @file Протокол ФР АТОЛ. */

// STL
#include <cmath>

// Project
#include "Atol2FR.h"
#include "Atol2FRConstants.h"

using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
char Atol2FRProtocol::calcCRC(const QByteArray & aData)
{
	if (!aData.size())
	{
		return 0;
	}

	char result = aData[0];

	for (int i = 1; i < aData.size(); ++i)
	{
		result ^= aData[i];
	}

	return result;
}

//--------------------------------------------------------------------------------
bool Atol2FRProtocol::unpack(QByteArray & aAnswer)
{
	// Минимальная длина
	if (aAnswer.size() < CAtol2FR::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("ATOL: The length of the packet is less than %1 byte").arg(CAtol2FR::MinAnswerSize));
		return false;
	}

	// Pr_KKM_4_15.pdf 14 страница Примечание 1
	QByteArray data;
	bool findPrefix = false;

	for (int i = 0; i < aAnswer.size() - 1; ++i)
	{
		// 1. ищем STX (все байты, неравные STX, игнорируем).
		if (aAnswer[i] == CAtol2FR::Prefix && !findPrefix)
		{
			findPrefix = true;
			continue;
		}
		else if (!findPrefix)
		{
			continue;
		}

		// 2. После STX все байты рассматривать как данные кадра.
		data.append(aAnswer[i]);

		if ((aAnswer[i + 1] == ASCII::DLE || aAnswer[i + 1] == ASCII::ETX) && aAnswer[i] == ASCII::DLE)
		{
			// маскируемый байт
			i++;
			data.append(aAnswer[i]);

			continue;
		}

		// 3. Принимать кадр до получения ETX.
		// 4. Если полученный байт ETX маскированный символом DLE, то
		// рассматривать его как часть данных и продолжать прием – п.3.
		if (aAnswer[i] == ASCII::ETX)
		{
			// 5. Принять 1 байт после немаскированного ETX – <CRC>.
			data.append(aAnswer[i + 1]);
			break;
		}
	}

	// Префикс
	if (!findPrefix)
	{
		toLog(LogLevel::Error, QString("ATOL: Failed to find prefix = %1").arg(toHexLog(CAtol2FR::Prefix)));
		return false;
	}

	// Длина
	if (data.size() < (CAtol2FR::MinAnswerSize - 1))
	{
		toLog(LogLevel::Error, QString("ATOL: The length of the final packet = %1, need %2 minimum").arg(data.size()).arg(CAtol2FR::MinAnswerSize - 1));
		return false;
	}

	// Постфикс
	char postfix = data[data.size() - 2];

	if (postfix != CAtol2FR::Postfix)
	{
		toLog(LogLevel::Error, QString("ATOL: Invalid postfix = %1, need = %2").arg(toHexLog(postfix)).arg(toHexLog(CAtol2FR::Postfix)));
		return false;
	}

	int dataSize = data.size() - 1;

	// CRC
	char CRC = calcCRC(data.left(dataSize));
	char answerCRC = data[dataSize];

	if (CRC != answerCRC)
	{
		toLog(LogLevel::Error, QString("ATOL: Invalid CRC of the answer = %1, need %2").arg(toHexLog(answerCRC)).arg(toHexLog(CRC)));
		return false;
	}

	aAnswer = data.left(dataSize - 1).replace(CAtol2FR::DLEMask, QByteArray(1, ASCII::DLE)).replace(CAtol2FR::ETXMask, QByteArray(1, ASCII::ETX));

	return true;
}

//--------------------------------------------------------------------------------
TResult Atol2FRProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, int aTimeout)
{
	QByteArray request(aCommandData);
	request.replace(ASCII::DLE, CAtol2FR::DLEMask).replace(ASCII::ETX, CAtol2FR::ETXMask);
	request.prepend(CAtol2FR::Password);
	request.append(CAtol2FR::Postfix);
	request.append(calcCRC(request));
	request.prepend(CAtol2FR::Prefix);

	toLog(LogLevel::Normal, QString("ATOL: >> {%1}").arg(request.toHex().data()));

	if (!execCommand(request))
	{
		return CommandResult::Transport;
	}

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

	if (!unpack(answer))
	{
		return CommandResult::Protocol;
	}

	aUnpackedAnswer = answer;

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool Atol2FRProtocol::execCommand(QByteArray & aPacket)
{
	if (!openWriteSession())
	{
		return false;
	}

	bool emptyAnswer = true;

	for (int i = 0; i < CAtol2FR::MaxRepeatPacket; ++i)
	{
		QByteArray answerData;

		if (!mPort->write(aPacket) || !mPort->read(answerData, CAtol2FR::Timeouts::OpeningSession))
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

			if (emptyAnswer && (i == (CAtol2FR::MaxRepeatPacket - 1)) && (answerData == QByteArray(answerData.size(), answerData[0])))
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
bool Atol2FRProtocol::read(QByteArray & aData)
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
			int size = aData.size();

			//если получили постфикс и перед ним не стоит маска, значит это постфикс и надо выходить
			if ((size > 2) &&
			    (aData[size - 2] == CAtol2FR::Postfix) &&
			    (aData[size - 3] != CAtol2FR::MaskByte))
			{
				break;
			}
		}
	}
	while (clockTimer.elapsed() < CAtol2FR::Timeouts::Default);

	return true;
}

//--------------------------------------------------------------------------------
bool Atol2FRProtocol::closeWriteSession()
{
	if (!mPort->write(QByteArray(1, ASCII::EOT)))
	{
		toLog(LogLevel::Error, "ATOL: Failed to close write session");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool Atol2FRProtocol::closeReadSession()
{
	QByteArray answerData;

	if (!mPort->read(answerData, CAtol2FR::Timeouts::Default) || !answerData.startsWith(ASCII::EOT))
	{
		toLog(LogLevel::Error, "ATOL: Failed to close read session");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool Atol2FRProtocol::sendACK()
{
	if (!mPort->write(QByteArray(1, ASCII::ACK)))
	{
		toLog(LogLevel::Error, "ATOL: Failed to send ACK");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool Atol2FRProtocol::openWriteSession()
{
	for (int i = 0; i < CAtol2FR::MaxServiceRequests; ++i)
	{
		QByteArray answerData;

		if (!mPort->write(QByteArray(1, ASCII::ENQ)) || !mPort->read(answerData, CAtol2FR::Timeouts::OpeningSession))
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

			SleepHelper::msleep(CAtol2FR::Timeouts::NAKOpeningSession);
		}
	}

	toLog(LogLevel::Error, "ATOL: Failed to open write session");

	closeWriteSession();

	return false;
}

//--------------------------------------------------------------------------------
bool Atol2FRProtocol::openReadSession(int aTimeout)
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
