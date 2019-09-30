/* @file Купюроприемник на протоколе SSP. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/CashAcceptors/CashAcceptorData.h"
#include "SSPCashAcceptor.h"
#include "SSPCashAcceptorConstants.h"
#include "SSPModelData.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;
using namespace ProtocolUtils;

//---------------------------------------------------------------------------
SSPCashAcceptor::SSPCashAcceptor()
{
	// параметры порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
	mPortParameters[EParameters::Parity].append(EParity::No);

	mPortParameters[EParameters::RTS].clear();
	mPortParameters[EParameters::RTS].append(ERTSControl::Disable);

	// данные устройства
	mDeviceName = CSSP::Models::Default;
	mEscrowPosition = 1;
	mResetWaiting = EResetWaiting::Available;
	mLastConnectionOK = false;
	mEnabled = false;
	mParInStacked = true;

	// параметры протокола
	mProtocol.setAddress(CSSP::Addresses::Validator);
	mDeviceCodeSpecification = PDeviceCodeSpecification(new CSSP::DeviceCodeSpecification);
}

//--------------------------------------------------------------------------------
QStringList SSPCashAcceptor::getModelList()
{
	CSSP::Models::CData modelData;
	QStringList result;

	foreach(auto data, modelData.data())
	{
		result << data.name;
	}

	return result;
}

//---------------------------------------------------------------------------------
bool SSPCashAcceptor::checkStatuses(TStatusData & aData)
{
	QByteArray answer;

	if (!processCommand(CSSP::Commands::Poll, &answer))
	{
		return false;
	}

	if (answer.isEmpty())
	{
		QByteArray statusCode = mEnabled ? CSSP::EnabledStatus : CSSP::DisabledStatus;
		aData << statusCode;
	}
	else
	{
		for (int i = 0; i < answer.size(); ++i)
		{
			aData << answer.mid(i, answer[i]);
			bool parAvailable = (answer[i] == CSSP::EAStatusCode) || (answer[i] == CSSP::StackingStarted);

			if (parAvailable && (++i < answer.size()))
			{
				aData.last() += answer[i];
			}
		}
	}

	return true;
}

//---------------------------------------------------------------------------------
TResult SSPCashAcceptor::execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	if (!mLastConnectionOK && (aCommand != QByteArray(1, CSSP::Commands::Sync)) && !processCommand(CSSP::Commands::Sync))
	{
		return false;
	}

	MutexLocker locker(&mExternalMutex);

	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	QByteArray answer;
	QByteArray request = aCommand + aCommandData;
	TResult result = mProtocol.processCommand(request, answer, CSSP::Commands::Data[aCommand[0]]);
	mLastConnectionOK = CommandResult::PresenceErrors.contains(result);

	if (!result)
	{
		return result;
	}

	CSSP::Result::SData resultData = CSSP::Result::Data[answer[0]];

	if (resultData.code != CommandResult::OK)
	{
		toLog(LogLevel::Error, QString("%1: %2").arg(mDeviceName).arg(resultData.description));
		return resultData.code;
	}

	if (aAnswer)
	{
		*aAnswer = answer.mid(1);
	}

	return CommandResult::OK;
}

//--------------------------------------------------------------------------------
bool SSPCashAcceptor::processReset()
{
	if (!processCommand(CSSP::Commands::Reset))
	{
		return false;
	}

	waitReady(CSSP::NotReadyWaiting, false);

	TStatusCodes statusCodes;
	auto poll = [&] () -> bool { statusCodes.clear(); return getStatus(std::ref(statusCodes)); };

	if (!PollingExpector().wait<bool>(poll, [&] () -> bool { return !statusCodes.isEmpty() && !statusCodes.contains(DeviceStatusCode::OK::Initialization); }, CSSP::ReadyWaiting))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to wait not initialization status after reset command");
		return false;
	}

	// иначе ITL забудет, по какому протоколу он работает
	int protocolNumber = 1;

	while (!processCommand(CSSP::Commands::SetProtocolVersion, QByteArray(1, uchar(protocolNumber++)))) {}
	while ( processCommand(CSSP::Commands::SetProtocolVersion, QByteArray(1, uchar(protocolNumber++)))) {}

	setDeviceParameter(CDeviceData::ProtocolVersion, protocolNumber - 2);

	return true;
}

//--------------------------------------------------------------------------------
bool SSPCashAcceptor::isConnected()
{
	QByteArray answer;

	if (!processCommand(CSSP::Commands::GetFirmware, &answer))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get firmware version");
		return false;
	}

	QMap<QString, SBaseModelData>::const_iterator it = CSSP::Models::Data.data().begin();
	SBaseModelData modelData = SBaseModelData(CSSP::Models::Default);

	while (it != CSSP::Models::Data.data().end())
	{
		if (answer.left(6).contains(it.key().toLatin1()))
		{
			modelData = it.value();
		}

		it++;
	}

	mDeviceName = modelData.name;
	mUpdatable  = modelData.updatable;
	mVerified   = modelData.verified;

	if (mUpdatable)
	{
		setDeviceParameter(CDeviceData::FirmwareUpdatable, true);
	}

	if (answer.size() >=  9)
	{
		setDeviceParameter(CDeviceData::Firmware, answer.mid(6, 3).insert(1, ASCII::Dot));
		setDeviceParameter(CDeviceData::CashAcceptors::ModificationNumber, answer.mid(9));
	}

	return true;
}

//--------------------------------------------------------------------------------
void SSPCashAcceptor::processDeviceData()
{
	QByteArray answer;

	if (processCommand(CSSP::Commands::GetSerial, &answer) && !answer.isEmpty())
	{
		setDeviceParameter(CDeviceData::SerialNumber, answer.toHex().toUInt(0, 16));
	}

	if (!containsDeviceParameter(CDeviceData::Firmware) && processCommand(CSSP::Commands::GetSetupData, &answer) && (answer.size() >= 5))
	{
		setDeviceParameter(CDeviceData::Firmware, answer.mid(1, 4).insert(1, ASCII::Dot));
	}

	if (processCommand(CSSP::Commands::GetDataset, &answer) && !answer.isEmpty())
	{
		setDeviceParameter(CDeviceData::CashAcceptors::BillSet, answer);
	}

	if (processCommand(CSSP::Commands::GetBuild, &answer) && !answer.isEmpty())
	{
		setDeviceParameter(CDeviceData::Build, "0x" + answer.toHex().toUpper());
		setDeviceParameter(CDeviceData::Type, uint(answer[0]), CDeviceData::Build);

		if (answer.size() >= 3)
		{
			ushort build = ushort(answer[1]) | ushort(answer[0] << 8);
			setDeviceParameter(CDeviceData::Number, build, CDeviceData::Build);
		}
	}
}

//--------------------------------------------------------------------------------
bool SSPCashAcceptor::setDefaultParameters()
{
	return true;
}

//---------------------------------------------------------------------------
bool SSPCashAcceptor::stack()
{
	if (!checkConnectionAbility() || (mInitialized != ERequestStatus::Success) || mCheckDisable)
	{
		return false;
	}

	// стека как такового нет, он происходит по поллу или если нет холда в течение 2 с.
	processCommand(CSSP::Commands::Poll);

	return true;
}

//---------------------------------------------------------------------------
bool SSPCashAcceptor::reject()
{
	if (!checkConnectionAbility() || (mInitialized == ERequestStatus::Fail))
	{
		return false;
	}

	return processCommand(CSSP::Commands::Return);
}

//---------------------------------------------------------------------------
bool SSPCashAcceptor::enableMoneyAcceptingMode(bool aEnabled)
{
	char command = aEnabled ? CSSP::Commands::Enable : CSSP::Commands::Disable;

	if (!processCommand(command))
	{
		return false;
	}

	mEnabled = aEnabled;

	return true;
}

//---------------------------------------------------------------------------
bool SSPCashAcceptor::applyParTable()
{
	QList<int> indexes = mEscrowParTable.data().keys();
	qSort(indexes);
	int channels = qCeil(indexes.last() / 8.0);

	if ((mDeviceName == CSSP::Models::NV200) || (mDeviceName == CSSP::Models::NV200Spectral))
	{
		channels = qMax(channels, CSSP::NV200MinInhibitedChannels);
	}

	QByteArray commandData(channels, ASCII::NUL);

	for (auto it = mEscrowParTable.data().begin(); it != mEscrowParTable.data().end(); ++it)
	{
		if (it->enabled && !it->inhibit)
		{
			int id = it.key() - indexes.first();
			int index = id / 8;
			commandData[index] = commandData[index] | (1 << id % 8);
		}
	}

	if (!processCommand(CSSP::Commands::SetInhibit, commandData))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to set nominals availability.");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool SSPCashAcceptor::loadParTable()
{
	QByteArray answer;

	if (!processCommand(CSSP::Commands::GetSetupData, &answer) || (answer.size() < 12))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get setup data");
		return false;
	}

	int channels = answer[11];
	int size = 16 + channels * 7;

	if (answer.size() < size)
	{
		toLog(LogLevel::Error, QString("%1: Failed to get setup data due to not enough data in the answer = %2, need %3.").arg(mDeviceName).arg(answer.size()).arg(size));
		return false;
	}

	double multiplier = CSSP::NominalMultiplier * hexToBCD(answer.mid(12 + 2 * channels, 3)).toInt();

	for (int i = 0; i < channels; ++i)
	{
		int index = answer[12 + i];
		QString currency = answer.mid(16 + 2 * channels, 3);
		double nominal = multiplier * revert(answer.mid(16 + 5 * channels + 4 * i, 4)).toHex().toInt(0, 16);

		MutexLocker locker(&mResourceMutex);

		mEscrowParTable.data().insert(index, SPar(nominal, currency));
	}

	return true;
}

//---------------------------------------------------------------------------
bool SSPCashAcceptor::performUpdateFirmware(const QByteArray & aBuffer)
{
	if (!performBaudRateChanging(true))
	{
		return false;
	}

	if (!performBaudRateChanging(false))
	{
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
bool SSPCashAcceptor::performBaudRateChanging(bool aUp)
{
	CSSP::EBaudRate::Enum upBaudRate = (mDeviceName == CSSP::Models::NV200Spectral) ? CSSP::EBaudRate::BR115200 : CSSP::EBaudRate::BR38400;
	CSSP::EBaudRate::Enum commandBaudrate = aUp ? upBaudRate : CSSP::EBaudRate::BR9600;
	IOPort::COM::EBaudRate::Enum baudRate = CSSP::BaudRateData[commandBaudrate];

	TPortParameters portParameters;
	mIOPort->getParameters(portParameters);

	if (portParameters[EParameters::BaudRate] == baudRate)
	{
		return true;
	}

	QByteArray commandData;
	commandData.append(char(commandBaudrate));
	commandData.append(CSSP::ContinuousBaudrate);

	if (!processCommand(CSSP::Commands::SetBaudrate, commandData))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to set %1 baud rate in cash acceptor").arg(baudRate));
		return false;
	}

	SleepHelper::msleep(CSSP::ChangingBaudratePause);

	portParameters[EParameters::BaudRate] = baudRate;

	if (!mIOPort->setParameters(portParameters))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to set %1 baud rate in port").arg(baudRate));
		return false;
	}

	toLog(LogLevel::Normal, mDeviceName + QString(": Baud rate has changed to %2.").arg(baudRate));

	return true;
}

//--------------------------------------------------------------------------------
