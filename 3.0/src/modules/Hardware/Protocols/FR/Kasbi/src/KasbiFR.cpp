/* @file Протокол ФР Касби. */

// STL
#include <cmath>

// Project
#include "KasbiFR.h"
#include "KasbiFRConstants.h"

//--------------------------------------------------------------------------------
ushort KasbiFRProtocol::calcCRC(const QByteArray & aData)
{
	uchar high = 0xFF;
	uchar low  = 0xFF;

	for (int i = 0; i < aData.size(); ++i)
	{
		uchar value = uchar(aData[i]) ^ high;
		ushort CRC = value << 8;

		for (int j = 0; j < 8; ++j) 
		{
			if (CRC & CKasbiFR::LastBit) 
			{
				CRC = (CRC << 1) ^ CKasbiFR::Polynominal;
			}
			else
			{
				CRC = CRC << 1;
			}
		}

		high = uchar(CRC >> 8) ^ low;
		low  = uchar(CRC);
	}

	return (ushort(high) << 8) + low;
}

//--------------------------------------------------------------------------------
bool KasbiFRProtocol::check(const QByteArray & aAnswer)
{
	// минимальный размер ответа
	if (aAnswer.size() < CKasbiFR::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("Kasbi: Invalid answer length = %1, need %2 minimum").arg(aAnswer.size()).arg(CKasbiFR::MinAnswerSize));
		return false;
	}

	// префикс
	QString prefix = QByteArray(CKasbiFR::Prefix).toHex().toUpper();
	QString answerPrefix = aAnswer.left(2).toHex().toUpper();

	if (prefix != answerPrefix)
	{
		toLog(LogLevel::Error, QString("Kasbi: Invalid prefix = 0x%1, need = 0x%2").arg(answerPrefix).arg(prefix));
		return false;
	}

	// длина
	int size = aAnswer.mid(2, 2).toHex().toUShort(0, 16);
	int answerSize = aAnswer.size() - 6;

	if (size != answerSize)
	{
		toLog(LogLevel::Error, QString("Kasbi: Invalid size = %1, need %2").arg(answerSize).arg(size));
		return false;
	}

	// CRC
	ushort CRC = calcCRC(aAnswer.mid(2, aAnswer.size() - 4));
	ushort answerCRC = qToBigEndian(aAnswer.right(2).toHex().toUShort(0, 16));

	if (CRC != answerCRC)
	{
		toLog(LogLevel::Error, QString("Kasbi: Invalid CRC = %1, need %2")
			.arg(ProtocolUtils::toHexLog(answerCRC))
			.arg(ProtocolUtils::toHexLog(CRC)));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
TResult KasbiFRProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, int aTimeout)
{
	QByteArray request = aCommandData;

	int size = aCommandData.size();
	request.prepend(char(size));
	request.prepend(char(size >> 8));

	ushort CRC = calcCRC(request);
	request.append(char(CRC));
	request.append(char(CRC >> 8));
	request.prepend(CKasbiFR::Prefix);

	toLog(LogLevel::Normal, QString("Kasbi: >> {%1}").arg(request.toHex().data()));
	QByteArray answer;

	if (!mPort->write(request) || !read(answer, aTimeout))
	{
		return CommandResult::Transport;
	}

	toLog(LogLevel::Normal, QString("Kasbi: << {%1}").arg(answer.toHex().data()));

	if (!check(answer))
	{
		toLog(LogLevel::Error, "Kasbi: Failed to unpack data");
		return CommandResult::Protocol;
	}

	aUnpackedAnswer = answer.mid(4, answer.size() - 6);

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool KasbiFRProtocol::read(QByteArray & aAnswer, int aTimeout)
{
	aAnswer.clear();

	QTime clockTimer;
	clockTimer.start();

	auto getChar = [&aAnswer] (int aIndex, int aShift) -> int { return int(uchar(aAnswer[aIndex])) << (8 * aShift); };

	do
	{
		QByteArray answerData;

		if (!mPort->read(answerData))
		{
			return false;
		}

		if (!answerData.isEmpty())
		{
			aAnswer.append(answerData);

			QString test = aAnswer.toHex();

			if (aAnswer.size() > 3)
			{
				int size = getChar(2, 1) | getChar(3, 0);

				if (aAnswer.size() >= (size + 6))
				{
					break;
				}
			}
		}
	}
	while (clockTimer.elapsed() < aTimeout);

	return true;
}

//--------------------------------------------------------------------------------
