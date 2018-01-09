/* @file Сторожевой таймер Platix. */

#include "Platix.h"

using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
Platix::Platix()
{
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);
	mPortParameters[EParameters::Parity].append(EParity::No);

	mDeviceName = "Platix";
}

//----------------------------------------------------------------------------
bool Platix::isConnected()
{
	return processCommand(CPlatix::Commands::GetID);
}

//----------------------------------------------------------------------------
bool Platix::reset(const QString & /*aLine*/)
{
	if (!checkConnectionAbility())
	{
		return false;
	}

	return processCommand(CPlatix::Commands::ResetModem);
}

//-----------------------------------------------------------------------------
void Platix::onPing()
{
	processCommand(CPlatix::Commands::Poll);
}

//--------------------------------------------------------------------------------
ushort Platix::calcCRC(const QByteArray & aData)
{
	ushort sum = 0;

	for (int i = 0; i < aData.size(); ++i)
	{
		sum += uchar(CPlatix::Sync - aData[i]);
	}

	return 256 - sum;
}

//--------------------------------------------------------------------------------
bool Platix::check(const QByteArray & aAnswer)
{
	if (aAnswer.size() < CPlatix::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("Platix: The length of the packet is less than %1 bytes").arg(CPlatix::MinAnswerSize));
		return false;
	}

	if (aAnswer[0] != CPlatix::Sync)
	{
		toLog(LogLevel::Error, QString("Platix: Invalid first byte (prefix) = %1, need %2")
			.arg(ProtocolUtils::toHexLog<char>(aAnswer[0])).arg(ProtocolUtils::toHexLog(CPlatix::Sync)));
		return false;
	}

	int length = uchar(aAnswer[1]);

	if (length < aAnswer.size())
	{
		toLog(LogLevel::Error, QString("Platix: Invalid length = %1, need %2").arg(aAnswer.size()).arg(length));
		return false;
	}

	ushort answerCRC = calcCRC(aAnswer.left(length - 1));
	ushort CRC = aAnswer[length - 1];

	if (answerCRC != CRC)
	{
		toLog(LogLevel::Error, QString("Platix: Invalid CRC = %1, need %2").arg(ProtocolUtils::toHexLog(answerCRC)).arg(ProtocolUtils::toHexLog(CRC)));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool Platix::processCommand(char aCommand)
{
	QByteArray request;
	request.append(CPlatix::Sync);
	request.append(CPlatix::CommandSize);
	request.append(aCommand);
	QString CRC = QString("%1").arg(calcCRC(request), 4, 16, QChar(ASCII::Zero));
	request.append(ProtocolUtils::getBufferFromString(CRC));

	if (!mIOPort->write(request))
	{
		return false;
	}

	if ((aCommand == CPlatix::Commands::RebootPC) || (aCommand == CPlatix::Commands::ResetModem))
	{
		return true;
	};

	QByteArray answer;

	//TODO: чтение данных с контролем длины
	if (!mIOPort->read(answer) || !check(answer))
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
