/* @file Протокол ФР ПРИМ. */

// STL
#include <numeric>

// Modules
#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Common/LoggingType.h"

// Project
#include "PrimFRConstants.h"
#include "PrimFR.h"

using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
PrimFRProtocol::PrimFRProtocol() : mDifferential(ASCII::Space), mLastCommandResult(0)
{
}

//--------------------------------------------------------------------------------
ushort PrimFRProtocol::calcCRC(const QByteArray & aData)
{
	return ushort(std::accumulate(aData.begin(), aData.end(), 0, [&] (ushort arg1, char arg2) -> ushort { return arg1 + uchar(arg2); }));
}

//--------------------------------------------------------------------------------
TResult PrimFRProtocol::check(const QByteArray & aRequest, const QByteArray & aAnswer, bool aPrinterMode)
{
	if (aAnswer.size() < CPrimFR::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("PRIM: The length of the packet is less than %1 byte").arg(CPrimFR::MinAnswerSize));
		return CommandResult::Protocol;
	}

	if (aAnswer[0] != CPrimFR::Prefix)
	{
		toLog(LogLevel::Error, QString("PRIM: Invalid prefix = %1, need = %2").arg(toHexLog(aAnswer[0])).arg(toHexLog(CPrimFR::Prefix)));
		return CommandResult::Protocol;
	}

	char postfix = aAnswer.right(5)[0];

	if (postfix != CPrimFR::Postfix)
	{
		toLog(LogLevel::Error, QString("PRIM: Invalid postfix = %1, need = %2").arg(toHexLog(postfix)).arg(toHexLog(CPrimFR::Postfix)));
		return CommandResult::Protocol;
	}

	if ((aAnswer[1] != char(mDifferential)) && !aPrinterMode)
	{
		toLog(LogLevel::Error, QString("PRIM: Invalid differential = %1, need = %2").arg(toHexLog(aAnswer[1])).arg(toHexLog(mDifferential)));
		return CommandResult::Id;
	}

	QString requestCommand = aRequest.mid(6, 2);
	QString  answerCommand = aAnswer.mid(2, 2);

	if ((requestCommand != answerCommand) && !aPrinterMode)
	{
		toLog(LogLevel::Error, QString("PRIM: Invalid command in answer = \"%1\", need = \"%2\"").arg(answerCommand).arg(requestCommand));
		return CommandResult::Id;
	}

	QByteArray unpackedData = aAnswer.left(aAnswer.size() - 4);
	ushort localCRC = calcCRC(unpackedData);
	bool isOK;
	ushort CRC = qToBigEndian(aAnswer.right(4).toUShort(&isOK, 16));

	if (!isOK)
	{
		toLog(LogLevel::Error, QString("PRIM: Filed to convert CRC = %1 from hex to digit").arg(aAnswer.right(4).data()));
		return CommandResult::CRC;
	}
	else if (localCRC != CRC)
	{
		toLog(LogLevel::Error, QString("PRIM: Invalid CRC = %1 (%2), need %3 (%4)").arg(CRC).arg(toHexLog(CRC)).arg(localCRC).arg(toHexLog(localCRC)));
		return CommandResult::CRC;
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
TResult PrimFRProtocol::processCommand(const QByteArray & aCommandData, QByteArray & aAnswer, int aTimeout)
{
	QByteArray request;

	request.append(CPrimFR::Prefix);
	request.append(CPrimFR::Password);

	mDifferential = (mDifferential == uchar(ASCII::Full)) ? ASCII::Space : ++mDifferential;

	request.append(mDifferential);
	request.append(aCommandData);

	request.append(CPrimFR::Separator);
	request.append(CPrimFR::Postfix);
	request.append(QString("%1").arg(qToBigEndian(calcCRC(request)), 4, 16, QChar(ASCII::Zero)).toUpper().toLatin1());

	TResult result = execCommand(request, aAnswer, aTimeout);

	if (result != CommandResult::NoAnswer)
	{
		return result;
	}

	int index = 0;

	do
	{
		if (index)
		{
			toLog(LogLevel::Normal, "PRIM: Command still working, sleep");
			SleepHelper::msleep(CPrimFR::CommandInProgressPause);
		}

		char answer;
		result = getCommandResult(answer, true);

		if (!result)
		{
			return result;
		}

		if (~answer & CPrimFR::CommandResultMask::PrinterError)
		{
			toLog(LogLevel::Normal, "PRIM: Printer error");
		}

		if (~answer & CPrimFR::CommandResultMask::PrinterMode)
		{
			toLog(LogLevel::Normal, "PRIM: Printer mode");
			aAnswer = QByteArray(1, answer);

			return CommandResult::NoAnswer;
		}

		if (~answer & CPrimFR::CommandResultMask::CommandVerified)
		{
			toLog(LogLevel::Normal, "PRIM: Command is not verified");
			return CommandResult::Transport;
		}

		if (answer & CPrimFR::CommandResultMask::CommandComplete)
		{
			toLog(LogLevel::Normal, "PRIM: Command done, trying to read");
			return execCommand(request, aAnswer, CPrimFR::DefaultTimeout, EPrimFRCommandConditions::NAKRepeat);
		}
	}
	while (++index < CPrimFR::RepeatingCount::CommandInProgress);

	return CommandResult::Transport;
}

//--------------------------------------------------------------------------------
TResult PrimFRProtocol::getCommandResult(char & aAnswer, bool aOnline)
{
	if (!aOnline && mLastCommandResult)
	{
		aAnswer = mLastCommandResult;

		return CommandResult::OK;
	}

	mRTProtocol.setPort(mPort);
	mRTProtocol.setLog(getLog());

	QVariantMap configuration;
	configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::ReadWrite));
	mPort->setDeviceConfiguration(configuration);

	TResult result = mRTProtocol.processCommand(0, aAnswer);
	mLastCommandResult = result ? aAnswer : 0;

	configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::None));
	mPort->setDeviceConfiguration(configuration);

	return result;
}

//--------------------------------------------------------------------------------
TResult PrimFRProtocol::execCommand(const QByteArray & aRequest, QByteArray & aAnswer, int aTimeout, EPrimFRCommandConditions::Enum aConditions)
{
	int index = 0;
	QByteArray request(aRequest);
	TResult result = CommandResult::OK;

	do
	{
		if (index || (aConditions == EPrimFRCommandConditions::NAKRepeat))
		{
			request = QByteArray(1, ASCII::NAK);
		}

		toLog(LogLevel::Normal, QString("PRIM: >> {%1}").arg(request.toHex().data()));
		aAnswer.clear();

		if (!mPort->write(request) || !readData(aAnswer, aTimeout))
		{
			return CommandResult::Port;
		}

		if (aAnswer.isEmpty())
		{
			return CommandResult::NoAnswer;
		}

		result = check(aRequest, aAnswer, aConditions == EPrimFRCommandConditions::PrinterMode);

		if (result)
		{
			aAnswer = aAnswer.mid(2, aAnswer.size() - 8);

			return CommandResult::OK;
		}
	}
	while(++index < CPrimFR::RepeatingCount::Protocol);

	return result;
}

//--------------------------------------------------------------------------------
bool PrimFRProtocol::readData(QByteArray & aData, int aTimeout)
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

		aData.append(data);
	}
	while (((aData.size() < CPrimFR::MinAnswerSize) || !aData.right(6).startsWith(CPrimFR::AnswerEndMark)) && (clockTimer.elapsed() < aTimeout));

	int index = aData.lastIndexOf(CPrimFR::Postfix);

	if (index != -1)
	{
		aData = aData.left(index + 5);
	}

	toLog(LogLevel::Normal, QString("PRIM: << {%1}").arg(aData.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
