/* @file Протокол ФР АТОЛ3. */

// STL
#include <cmath>

// Project
#include "Atol3FR.h"
#include "Atol3FRConstants.h"

using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
Atol3FRProtocol::Atol3FRProtocol(): mId(ASCII::NUL)
{
}

//--------------------------------------------------------------------------------
char Atol3FRProtocol::calcCRC(const QByteArray & aData)
{
	char CRC = CAtol3FR::CRC::StartValue;

	for (int i = 0; i < aData.size(); ++i)
	{
		CRC ^= aData[i];

		for (int j = 0; j < 8; ++j)
		{
			if (CRC & CAtol3FR::CRC::LastBit)
			{
				CRC = (CRC << 1) ^ CAtol3FR::CRC::Polynominal;
			}
			else
			{
				CRC <<= 1;
			}
		}
	}

	return CRC;
}

//--------------------------------------------------------------------------------
bool Atol3FRProtocol::check(const QByteArray & aAnswer)
{
	// минимальный размер ответа
	if (aAnswer.size() < CAtol3FR::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("Kasbi: Invalid answer length = %1, need %2 minimum").arg(aAnswer.size()).arg(CAtol3FR::MinAnswerSize));
		return false;
	}

	// префикс
	if (CAtol3FR::Prefix != aAnswer[0])
	{
		toLog(LogLevel::Error, QString("ATOL3: Invalid prefix = %1, need = %2").arg(toHexLog(aAnswer[0])).arg(toHexLog(CAtol3FR::Prefix)));
		return false;
	}

	// Id
	if ((aAnswer[3] != char(mId)) && (aAnswer[3] != CAtol3FR::AsyncId))
	{
		toLog(LogLevel::Error, QString("ATOL3: Invalid Id = %1, need = %2").arg(toHexLog(aAnswer[3])).arg(toHexLog(mId)));
		return false;
	}

	// длина
	int size = qToBigEndian(aAnswer.mid(1, 2).toHex().toUShort(0, 16));
	int answerSize = aAnswer.size() - 5;

	if (size != answerSize)
	{
		toLog(LogLevel::Error, QString("ATOL3: Invalid size = %1, need %2").arg(answerSize).arg(size));
		return false;
	}

	// CRC
	char CRC = calcCRC(aAnswer.mid(3, aAnswer.size() - 4));
	char answerCRC = aAnswer[aAnswer.size() - 1];

	if (CRC != answerCRC)
	{
		toLog(LogLevel::Error, QString("ATOL3: Invalid CRC = %1, need %2")
			.arg(toHexLog(answerCRC))
			.arg(toHexLog(CRC)));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
TResult Atol3FRProtocol::execCommand(char aCommand, const QByteArray & aCommandData, QByteArray & aAnswer, int aTimeout)
{
	mId = (mId == uchar(CAtol3FR::LastId)) ? ASCII::NUL : ++mId;
	QByteArray request = QByteArray(1, char(mId)) + aCommand + aCommandData;

	ushort size = ushort(request.size() - 1);
	request.prepend(char(size >> 7));
	request.prepend(char(size) & CAtol3FR::SizeMask);

	request.prepend(CAtol3FR::Prefix);
	request.append(calcCRC(request.mid(3)));

	request = request.left(4) + request.mid(4).replace(CAtol3FR::Prefix, CAtol3FR::ESCTSTX).replace(CAtol3FR::ESC, CAtol3FR::ESCTESC);

	toLog(LogLevel::Normal, QString("ATOL3: >> {%1}").arg(request.toHex().data()));

	if (!mPort->write(request))
	{
		return CommandResult::Transport;
	}

	return waitForAnswer(aAnswer);
}

//--------------------------------------------------------------------------------
TResult Atol3FRProtocol::waitForAnswer(QByteArray & aUnpackedAnswer)
{
	QByteArray answer;

	if (!read(answer, CAtol3FR::Timeouts::GetResult))
	{
		return CommandResult::Transport;
	}

	answer.replace(CAtol3FR::ESCTSTX, QByteArray(1, CAtol3FR::Prefix)).replace(CAtol3FR::ESCTESC, QByteArray(1, CAtol3FR::ESC));

	if (answer.isEmpty())
	{
		return CommandResult::NoAnswer;
	}
	else if (!check(answer))
	{
		return CommandResult::Protocol;
	}

	aUnpackedAnswer = answer.mid(4, answer.size() - 5);

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
TResult Atol3FRProtocol::processCommand(uchar aTId, const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, int aTimeout)
{
	QByteArray commandData = CAtol3FR::TaskFlags::NeedResult + QByteArray(1, char(aTId)) + CAtol3FR::Password + aCommandData;

	return execCommand(CAtol3FR::Commands::Add, commandData, aUnpackedAnswer, aTimeout);
}

//--------------------------------------------------------------------------------
TResult Atol3FRProtocol::getResult(uchar aTId, QByteArray & aUnpackedAnswer)
{
	return execCommand(CAtol3FR::Commands::Req, QByteArray(1, char(aTId)), aUnpackedAnswer, CAtol3FR::Timeouts::GetResult);
}

//--------------------------------------------------------------------------------
TResult Atol3FRProtocol::sendACK(uchar aTId)
{
	QByteArray commandData;

	return execCommand(CAtol3FR::Commands::ACK, QByteArray(1, char(aTId)), commandData, CAtol3FR::Timeouts::ACK);
}

//--------------------------------------------------------------------------------
TResult Atol3FRProtocol::cancel()
{
	QByteArray commandData;
	QByteArray answer;

	return execCommand(CAtol3FR::Commands::Cancel, commandData, answer, CAtol3FR::Timeouts::Cancel);
}

//--------------------------------------------------------------------------------
bool Atol3FRProtocol::read(QByteArray & aAnswer, int aTimeout)
{
	QTime clockTimer;
	clockTimer.start();

	do
	{
		QByteArray data;

		if (!mPort->read(data, 30))
		{
			return false;
		}

		if (!data.isEmpty())
		{
			aAnswer.append(data);

			if (aAnswer.size() > 2)
			{
				int start = aAnswer.lastIndexOf(CAtol3FR::Prefix);
				ushort size = ushort(aAnswer[start + 1] & CAtol3FR::SizeMask) | (ushort(uchar(aAnswer[start + 2])) << 7);

				if (aAnswer.size() >= (size + 5))
				{
					break;
				}
			}
		}
	}
	while (clockTimer.elapsed() < aTimeout);

	QString log = aAnswer.toHex();
	QString hexPrefix = QByteArray(1, CAtol3FR::Prefix).toHex();
	log.replace(hexPrefix, " " + hexPrefix);
	log = QString("ATOL3: << {%1}").arg(log.simplified());

	int index = aAnswer.lastIndexOf(CAtol3FR::Prefix);

	if (index)
	{
		aAnswer = aAnswer.mid(index);
		log += QString(" -> {%1}").arg(aAnswer.toHex().data());
	}

	toLog(LogLevel::Normal, QString("ATOL3: << {%1}").arg(aAnswer.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
