/* @file Протокол ФР АТОЛ3. */

// STL
#include <cmath>

// Project
#include "Atol3FR.h"
#include "Atol3FRConstants.h"

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
void Atol3FRProtocol::pack(QByteArray & aData)
{
	aData.prepend(char(mId));
	mId = (mId == uchar(CAtol3FR::LastId)) ? ASCII::NUL : ++mId;

	ushort size = ushort(aData.size() - 1);
	aData.prepend(char(size >> 7));
	aData.prepend(char(size) & CAtol3FR::SizeMask);

	aData.prepend(CAtol3FR::STX);
	aData.append(calcCRC(aData.mid(3)));

	aData = aData.left(4) + aData.mid(4).replace(CAtol3FR::STX, CAtol3FR::ESCTSTX).replace(CAtol3FR::ESC, CAtol3FR::ESCTESC);
}

//--------------------------------------------------------------------------------
bool Atol3FRProtocol::check(const QByteArray & aAnswer)
{
	return true;
}

//--------------------------------------------------------------------------------
TResult Atol3FRProtocol::execCommand(char aCommand, QByteArray & aAnswer, int aTimeout)
{
	QByteArray commandData;

	return execCommand(aCommand, commandData, aAnswer, aTimeout);
}

//--------------------------------------------------------------------------------
TResult Atol3FRProtocol::execCommand(char aCommand, const QByteArray & aCommandData, QByteArray & aAnswer, int aTimeout)
{
	QByteArray request(1, aCommand);
	request.append(aCommandData);
	pack(request);

	toLog(LogLevel::Normal, QString("ATOL3: >> {%1}").arg(request.toHex().data()));

	QByteArray answer;

	if (!mPort->write(request) || !read(answer, aTimeout))
	{
		return CommandResult::Transport;
	}

	if (answer.isEmpty())
	{
		if (aCommand != CAtol3FR::Commands::Cancel)
		{
			TResult result = execCommand(CAtol3FR::Commands::Cancel, answer, CAtol3FR::Timeouts::Cancel);

			return result ? CommandResult::Answer : result;
		}

		return CommandResult::NoAnswer;
	}
	else if (!check(answer))
	{
		return CommandResult::Protocol;
	}

	aAnswer = answer.mid(4, answer.size() - 5);

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
TResult Atol3FRProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, int aTimeout)
{
	return execCommand(CAtol3FR::Commands::Add, CAtol3FR::TaskFlags::NeedResult + aCommandData, aUnpackedAnswer, aTimeout);
}

//--------------------------------------------------------------------------------
TResult Atol3FRProtocol::getResult(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer)
{
	return execCommand(CAtol3FR::Commands::Req, aCommandData, aUnpackedAnswer, CAtol3FR::Timeouts::GetResult);
}

//--------------------------------------------------------------------------------
bool Atol3FRProtocol::read(QByteArray & aAnswer, int aTimeout)
{
	QTime clockTimer;
	clockTimer.start();

	do
	{
		QByteArray data;

		if (!mPort->read(data))
		{
			return false;
		}

		if (!data.isEmpty())
		{
			aAnswer.append(data);

			if (aAnswer.size() > 2)
			{
				ushort size = ushort(aAnswer[1] & CAtol3FR::SizeMask) | (ushort(uchar(aAnswer[2])) << 7);

				if (aAnswer.size() >= (size + 5))
				{
					break;
				}
			}
		}
	}
	while (clockTimer.elapsed() < aTimeout);

	toLog(LogLevel::Normal, QString("ATOL3: << {%1}").arg(aAnswer.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
