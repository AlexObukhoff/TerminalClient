/* @file Диспенсер купюр Puloon. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "PuloonCashDispenser.h"
#include "PuloonCashDispenserData.h"
#include "PuloonModelData.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
PuloonLCDM::PuloonLCDM()
{
	// Параметры порта.
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);    // default
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);

	mPortParameters[EParameters::Parity].append(EParity::No);

	// данные устройства
	mDeviceName = CPuloonLCDM::Models::Data.getDefault().name;
}

//---------------------------------------------------------------------------
QStringList PuloonLCDM::getModelList()
{
	CPuloonLCDM::Models::CData modelData;
	QStringList result;

	foreach (const CPuloonLCDM::Models::SData & data, modelData.data().values())
	{
		result << data.name;
	}

	return result;
}

//--------------------------------------------------------------------------------
TResult PuloonLCDM::processCommand(char aCommand, QByteArray * aAnswer)
{
	QByteArray commandData;

	return processCommand(aCommand, commandData, aAnswer);
}

//--------------------------------------------------------------------------------
TResult PuloonLCDM::processCommand(char aCommand, const QByteArray & aCommandData, QByteArray * aAnswer, int aTimeout)
{
	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	QByteArray commandData = QByteArray(1, CPuloonLCDM::CommunicationId) + aCommand + aCommandData;
	CPuloonLCDM::Commands::SData commandInfo = CPuloonLCDM::Commands::Data[aCommand];
	int timeout = aTimeout ? aTimeout : commandInfo.timeout;

	TAnswerList answerList;
	TResult result = mProtocol.processCommand(commandData, answerList, timeout);

	if (!result)
	{
		return result;
	}

	if (answerList.isEmpty() || (answerList.last().size() == CPuloon::ItemDataSize))
	{
		return CommandResult::Answer;
	}

	foreach (const QByteArray & data, answerList)
	{
		if ((data.size() == CPuloon::ItemDataSize) && (data[1] == CPuloonLCDM::RejectingID))
		{
			toLog(LogLevel::Warning, QString("%1: rejecting, reason = %2").arg(mDeviceName).arg(CPuloonLCDM::RejectingSpecification[data[2]]));
		}
	}

	if (aAnswer)
	{
		*aAnswer = answerList.last();
	}

	QByteArray & answerData = answerList.last();

	if (answerData.size() < CPuloonLCDM::MinAnswerDataSize)
	{
		toLog(LogLevel::Error, QString("%1: Answer data size = %2, need min %3").arg(mDeviceName).arg(answerData.size()).arg(CPuloonLCDM::MinAnswerDataSize));
		return CommandResult::Answer;
	}

	if (commandInfo.error != -1)
	{
		if (answerData.size() < (commandInfo.error + 1))
		{
			toLog(LogLevel::Error, QString("%1: Answer data size = %2, need min %3 for error handling").arg(mDeviceName).arg(answerData.size()).arg(commandInfo.error));
			return CommandResult::Answer;
		}

		char lastDeviceCode = answerData[commandInfo.error];
		SDeviceCodeSpecification deviceCodeData = CPuloonLCDM::DeviceCodeSpecification[lastDeviceCode];
		EWarningLevel::Enum warningLevel = mStatusCodesSpecification->value(deviceCodeData.statusCode).warningLevel;

		if (aCommand != CPuloonLCDM::Commands::GetStatus)
		{
			if (warningLevel == EWarningLevel::Error)
			{
				toLog(LogLevel::Error, QString("%1: %2").arg(mDeviceName).arg(deviceCodeData.description));
				return CommandResult::Answer;
			}
			else if (warningLevel == EWarningLevel::Warning)
			{
				toLog(LogLevel::Warning, QString("%1: %2").arg(mDeviceName).arg(deviceCodeData.description));
			}
		}
	}

	return CommandResult::OK;
}

//---------------------------------------------------------------------------
bool PuloonLCDM::getStatus(TStatusCodes & aStatusCodes)
{
	QByteArray answer;

	if (!processCommand(CPuloonLCDM::Commands::GetStatus, &answer))
	{
		return false;
	}

	QByteArray deviceStatusCodes = answer.right(3);
	SDeviceCodeSpecification statusSpecification = CPuloonLCDM::DeviceCodeSpecification[deviceStatusCodes[0]];
	aStatusCodes.insert(statusSpecification.statusCode);

	TDeviceCodeSpecifications sensorSpecifications;
	CPuloonLCDM::SensorSpecification.getSpecification(deviceStatusCodes.mid(1), sensorSpecifications);

	foreach (const SDeviceCodeSpecification & specification, sensorSpecifications)
	{
		aStatusCodes.insert(specification.statusCode);
	}

	if (deviceStatusCodes != mLastDeviceStatusCodes)
	{
		logStatusChanging(statusSpecification, sensorSpecifications);
	}

	mLastDeviceStatusCodes = deviceStatusCodes;

	return true;
}

//--------------------------------------------------------------------------------
void PuloonLCDM::logStatusChanging(const SDeviceCodeSpecification & aStatusSpecification, const TDeviceCodeSpecifications & aSensorSpecifications)
{
	EWarningLevel::Enum warningLevel = EWarningLevel::OK;
	QString sensorLog;

	foreach (const SDeviceCodeSpecification & specification, aSensorSpecifications)
	{
		if (!specification.description.isEmpty())
		{
			sensorLog += (sensorLog.isEmpty() ? "" : ", ") + specification.description;
		}

		warningLevel = qMax(warningLevel, mStatusCodesSpecification->value(specification.statusCode).warningLevel);
	}

	bool status  = !aStatusSpecification.description.isEmpty();
	bool sensors = !sensorLog.isEmpty();

	if (status || sensors)
	{
		LogLevel::Enum logLevel = (warningLevel == EWarningLevel::Error) ? LogLevel::Error :
			((warningLevel == EWarningLevel::Warning) ? LogLevel::Warning : LogLevel::Normal);
		toLog(logLevel, QString("%1: %2%3%4")
			.arg(mDeviceName)
			.arg(status ? "status: " + aStatusSpecification.description : "")
			.arg((status && sensors) ? ", " : "")
			.arg(sensors ? "sensors: " + sensorLog : ""));
	}
}

//--------------------------------------------------------------------------------
bool PuloonLCDM::reset()
{
	return processCommand(CPuloonLCDM::Commands::Reset);
}

//--------------------------------------------------------------------------------
bool PuloonLCDM::isConnected()
{
	QByteArray answer;

	if (!processCommand(CPuloonLCDM::Commands::GetROMVersion, &answer))
	{
		return false;
	}

	QString textCheckSum = answer.mid(4, 4).toHex().remove(0, 1).remove(1, 1).remove(2, 1).remove(3, 1).toUpper();
	setDeviceParameter(CDeviceData::CheckSum, textCheckSum);

	bool checkSumOK;
	ushort checkSum = textCheckSum.toUShort(&checkSumOK, 16);

	CPuloonLCDM::Models::SData modelData;

	if (checkSumOK)
	{
		modelData = CPuloonLCDM::Models::Data[checkSum];
		mDeviceName = modelData.name;
		mUnits = modelData.units;
	}

	QString textROMVersion = answer.mid(1, 2);
	bool ROMVersionOK;
	int ROMVersion = textROMVersion.toInt(&ROMVersionOK);
	QString resultROMVersion = textROMVersion;

	if (ROMVersionOK && modelData.ROMVersion && (ROMVersion == modelData.ROMVersion))
	{
		resultROMVersion = modelData.fullROMVersion;
	}

	mVerified = modelData.verified;

	setDeviceParameter(CDeviceData::Firmware, resultROMVersion);

	return true;
}

//--------------------------------------------------------------------------------
void PuloonLCDM::performDispense(int aUnit, int aItems)
{
	if (!isWorkingThread())
	{
		QMetaObject::invokeMethod(this, "performDispense", Qt::QueuedConnection, Q_ARG(int, aUnit), Q_ARG(int, aItems));

		return;
	}

	int starts = qCeil(double(aItems)/CPuloonLCDM::MaxOutputPack);
	int maxFullPackAmount = (starts - 1) * CPuloonLCDM::MaxOutputPack;
	int dispensedItems = 0;

	for (int i = 0; i < starts; ++i)
	{
		int pack = ((i + 1) == starts) ? aItems - maxFullPackAmount : CPuloonLCDM::MaxOutputPack;
		QByteArray commandData = QString("%1").arg(pack, 2, 10, QChar(ASCII::Zero)).toLatin1();

		char command = aUnit ? CPuloonLCDM::Commands::LowerDispense : CPuloonLCDM::Commands::UpperDispense;
		int timeout = CPuloonLCDM::MaxNoteDispensingTimeout * pack + CPuloonLCDM::MaxTimeoutEmptyUnit;
		QByteArray answer;

		TResult result = processCommand(command, commandData, &answer, timeout);

		if (result || (result == CommandResult::Answer))
		{
			if ((answer.size() > 4) && ((answer[4] == CPuloonLCDM::UpperUnitEmpty) || (answer[4] == CPuloonLCDM::LowerUnitEmpty)))
			{
				emitUnitEmpty(aUnit);
			}

			dispensedItems += answer.mid(2, 2).toInt();
			int rejectedItems = answer.mid(6, 2).toInt();

			if (rejectedItems)
			{
				toLog(LogLevel::Warning, mDeviceName + QString(": emit rejected %1 notes from %2 unit").arg(rejectedItems).arg(aUnit));

				emit rejected(aUnit, rejectedItems);
			}
		}

		if (!result)
		{
			break;
		}
	}

	emitDispensed(aUnit, dispensedItems);
}

//--------------------------------------------------------------------------------
