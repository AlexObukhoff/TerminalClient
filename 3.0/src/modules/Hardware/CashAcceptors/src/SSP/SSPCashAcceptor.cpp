/* @file Купюроприемник на протоколе SSP. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/CashAcceptors/CashAcceptorData.h"
#include "SSPCashAcceptor.h"
#include "SSPCashAcceptorConstants.h"
#include "SSPModelData.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
SSPCashAcceptor::SSPCashAcceptor()
{
	// параметры порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
	mPortParameters[EParameters::Parity].append(EParity::No);

	mPortParameters[EParameters::RTS].clear();
	mPortParameters[EParameters::RTS].append(ERTSControl::Disable);

	// данные устройства
	mDeviceName = "SSP cash acceptor";
	mEscrowPosition = 1;
	mResetOnIdentification = true;
	mResetWaiting = EResetWaiting::Available;

	// параметры протокола
	mProtocol.setAddress(CSSP::Addresses::Validator);
	mDeviceCodeSpecification = PDeviceCodeSpecification(new CSSP::DeviceCodeSpecification);
}

//--------------------------------------------------------------------------------
QStringList SSPCashAcceptor::getModelList()
{
	CSSP::CModelData modelData;
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
		aData << CSSP::EnabledStatus;
	}
	else
	{
		for (int i = 0; i < answer.size(); ++i)
		{
			TStatusCodes statusCodes;
			mDeviceCodeSpecification->getSpecification(answer[i], statusCodes);
			int addon = int(statusCodes.contains(BillAcceptorStatusCode::BillOperation::Escrow));

			aData << answer.mid(i, 1 + addon);
			i += addon;
		}
	}

	return true;
}

//---------------------------------------------------------------------------------
TResult SSPCashAcceptor::execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	MutexLocker locker(&mExternalMutex);

	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	QByteArray answer;
	QByteArray request = aCommand + aCommandData;
	TResult result = mProtocol.processCommand(request, answer, aCommand.startsWith(CSSP::Commands::Sync));

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
	return processCommand(CSSP::Commands::Reset) && waitReady(CSSP::ReadyWaiting);
}

//--------------------------------------------------------------------------------
bool SSPCashAcceptor::isConnected()
{
	QByteArray answer;

	if (!processCommand(CSSP::Commands::Sync))
	{
		return false;
	}

	if (!processReset())
	{
		return false;
	}

	int protocolNumber = CSSP::StartingProtocolNumber;

	while (processCommand(CSSP::Commands::SetProtocolVersion, QByteArray(1, uchar(protocolNumber++))))
	{
	}

	setDeviceParameter(CDeviceData::ProtocolVersion, protocolNumber - 2);

	if (!processCommand(CSSP::Commands::GetVersion, &answer))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get firmware version");
		return false;
	}

	SBaseModelData modelData = CSSP::ModelData[answer.left(6)];
	mDeviceName = modelData.name;
	mUpdatable  = modelData.updatable;
	mVerified   = modelData.verified;

	if (answer.size() >=  9)
	{
		setDeviceParameter(CDeviceData::Firmware, answer.mid(6, 3).insert(1, ASCII::Dot));
	}

	setDeviceParameter(CDeviceData::Revision, answer.mid(9, 4));
	setDeviceParameter(CDeviceData::CashAcceptors::ModificationNumber, answer.mid(13));

	if (processCommand(CSSP::Commands::GetSerial, &answer) && !answer.isEmpty())
	{
		setDeviceParameter(CDeviceData::SerialNumber, answer.toHex().toUInt(0, 16));
	}

	if (!containsDeviceParameter(CDeviceData::Firmware) && processCommand(CSSP::Commands::GetSetupData, &answer) && (answer.size() >= 5))
	{
		setDeviceParameter(CDeviceData::Firmware, answer.mid(1, 4).insert(1, ASCII::Dot));
	}

	return true;
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

	return processCommand(CSSP::Commands::Stack);
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

	return processCommand(command);
}

//---------------------------------------------------------------------------
bool SSPCashAcceptor::applyParTable()
{
	QList<int> indexes = mEscrowParTable.data().keys();
	qSort(indexes);
	int channels = qCeil(indexes.last() / 8.0);
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

	double multiplier = CSSP::NominalMultiplier * ProtocolUtils::hexToBCD(answer.mid(12 + 2 * channels, 3)).toInt();

	for (int i = 0; i < channels; ++i)
	{
		int index = answer[12 + i];
		QString currency = answer.mid(16 + 2 * channels, 3);
		double nominal = multiplier * ProtocolUtils::revert(answer.mid(16 + 5 * channels + 4 * i, 4)).toHex().toInt(0, 16);

		MutexLocker locker(&mResourceMutex);

		mEscrowParTable.data().insert(index, SPar(nominal, currency));
	}

	return true;
}

//--------------------------------------------------------------------------------
