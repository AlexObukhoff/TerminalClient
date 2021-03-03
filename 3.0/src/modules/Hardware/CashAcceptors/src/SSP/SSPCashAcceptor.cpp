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
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
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
	mFirmware = 0;
	mUpdatable = true;

	setDeviceParameter(CDeviceData::FirmwareUpdatable, mUpdatable);

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
	TResult result = mProtocol.processCommand(request, answer, CSSP::Commands::Data[aCommand]);
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

	// для анализа выхода из инициализации.
	poll();

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

	QMap<QString, CSSP::Models::SData>::const_iterator it = CSSP::Models::Data.data().begin();
	mModelData = CSSP::Models::SData(CSSP::Models::Default);

	while (it != CSSP::Models::Data.data().end())
	{
		if (answer.left(6).contains(it.key().toLatin1()))
		{
			mModelData = it.value();
		}

		it++;
	}

	mDeviceName = mModelData.name;
	mVerified   = mModelData.verified;
	mOldFirmware = false;
	mFirmware = 0;

	if (answer.size() >=  9)
	{
		mFirmware = answer.mid(6, 3).insert(1, ASCII::Dot).toDouble();

		setDeviceParameter(CDeviceData::Firmware, answer.mid(6, 3).insert(1, ASCII::Dot));
		setDeviceParameter(CDeviceData::CashAcceptors::ModificationNumber, answer.mid(9));

		if (mFirmware)
		{
			mOldFirmware = mFirmware < mModelData.lastFirmware;
		}
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

	setPortDeviceName(ProtocolNames::CashDevice::SSP);
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

	if ((mDeviceName == CSSP::Models::NV200ST) || (mDeviceName == CSSP::Models::NV200Spectral))
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
	QByteArray Id = aBuffer.left(3);

	if (Id != CSSP::UpdatingFirmware::Id)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to update firmware due to the firmware buffer starts with wrong Id = %1 (0x%2), need %3.")
			.arg(Id.data()).arg(Id.toHex().toUpper().data()).arg(CSSP::UpdatingFirmware::Id));
		return false;
	}

	TPortParameters portParameters;
	mIOPort->getParameters(portParameters);

	if (!performBaudRateChanging(true))
	{
		return false;
	}

	QByteArray data;

	if (!processCommand(CSSP::Commands::UpdatingFirmware::GetBlockSize, &data))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to get block size");
		return false;
	}

	if (data.size() < 2)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to get block size due to data size = %1, need 2 min").arg(data.size()));
		return false;
	}

	int blockSize = revert(data.left(2)).toHex().toInt(0, 16);
	QByteArray header = aBuffer.left(CSSP::UpdatingFirmware::HeaderSize);

	if (!processCommand(header))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to write header");
		return false;
	}

	setPortLoggingType(ELoggingType::ReadWrite);

	int RAMSize = aBuffer.mid(7, 4).toHex().toInt(0, 16);
	QByteArray RAMData = aBuffer.mid(CSSP::UpdatingFirmware::HeaderSize, RAMSize);

	if (!writeRAMData(RAMData))
	{
		return false;
	}

	if (!mIOPort->write(QByteArray(1, aBuffer[6])) || !mIOPort->read(data, CSSP::UpdatingFirmware::DefaultTimeout) || data.isEmpty() || (data[0] != CSSP::UpdatingFirmware::ACK))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to write update code");
		return false;
	}

	if (!mIOPort->write(header) || !mIOPort->read(data, CSSP::UpdatingFirmware::DefaultTimeout) || data.isEmpty() || (data[0] != CSSP::UpdatingFirmware::ACK))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to write header directly");
		return false;
	}

	QByteArray datasetData = aBuffer.mid(CSSP::UpdatingFirmware::HeaderSize + RAMSize);

	if (!writeDatasetData(datasetData, blockSize))
	{
		return false;
	}

	setPortLoggingType(mIOMessageLogging);

	mIOPort->close();
	SleepHelper::msleep(CSSP::UpdatingFirmware::Pause::EndApplying);
	mIOPort->open();

	portParameters[EParameters::BaudRate] = IOPort::COM::EBaudRate::BR9600;
	mIOPort->setParameters(portParameters);

	mLastConnectionOK = false;
	waitReady(CSSP::UpdatingFirmware::FinalizationWaiting);

	return true;
}

//---------------------------------------------------------------------------
char SSPCashAcceptor::getCRC(const QByteArray & aData)
{
	char CRC = ASCII::NUL;

	for (int i = 0; i < aData.size(); ++i)
	{
		CRC ^= aData[i];
	}

	return CRC;
}

//---------------------------------------------------------------------------
bool SSPCashAcceptor::writeRAMData(const QByteArray & aData)
{
	int RAMSize = aData.size();
	int blocks = qCeil(double(RAMSize) / CSSP::UpdatingFirmware::BlockSize);

	toLog(LogLevel::Normal, mDeviceName + QString(": Writing RAM blocks: total = %1, block size = %2, RAM size = %3").arg(blocks).arg(CSSP::UpdatingFirmware::BlockSize).arg(RAMSize));

	for (int i = 0; i < blocks; ++i)
	{
		SleepHelper::msleep(CSSP::UpdatingFirmware::Pause::BlockWriting);
		QByteArray block = aData.mid(i * CSSP::UpdatingFirmware::BlockSize, CSSP::UpdatingFirmware::BlockSize);

		if (!mIOPort->write(block))
		{
			toLog(LogLevel::Error, mDeviceName + QString(": Failed to write %1 RAM block").arg(i));
			return false;
		}
	}

	QByteArray data;

	if (!mIOPort->read(data, CSSP::UpdatingFirmware::DefaultTimeout) || data.isEmpty())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to process reading CRC for RAM data");
		return false;
	}

	char CRC = getCRC(aData);

	if (data[0] != CRC)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to check CRC for RAM data = %1, need %2").arg(toHexLog(data.at(0))).arg(toHexLog(CRC)));
		return false;
	}

	SleepHelper::msleep(CSSP::UpdatingFirmware::Pause::RAMApplying);

	return true;
}

//---------------------------------------------------------------------------
bool SSPCashAcceptor::writeDatasetData(const QByteArray & aData, int aBlockSize)
{
	int datasetSize = aData.size();
	int blocks = qCeil(double(datasetSize) / aBlockSize);

	toLog(LogLevel::Normal, mDeviceName + QString(": Writing dataset blocks: total = %1, block size = %2, dataset size = %3").arg(blocks).arg(aBlockSize).arg(datasetSize));

	for (int i = 0; i < blocks; ++i)
	{
		SleepHelper::msleep(CSSP::UpdatingFirmware::Pause::BlockWriting);
		QByteArray block = aData.mid(i * aBlockSize, aBlockSize);

		if (!mIOPort->write(block))
		{
			toLog(LogLevel::Error, mDeviceName + QString(": Failed to write %1 dataset block").arg(i));
			return false;
		}

		char CRC = getCRC(block);
		QByteArray data;

		if (!mIOPort->write(QByteArray(1, CRC)) || !mIOPort->read(data, CSSP::UpdatingFirmware::DefaultTimeout) || data.isEmpty())
		{
			toLog(LogLevel::Error, mDeviceName + QString(": Failed to process CRC writing of %1 dataset block").arg(i));
			return false;
		}

		if (data[0] != CRC)
		{
			toLog(LogLevel::Error, mDeviceName + QString(": Failed to check CRC for dataset block %1 = %2, need %3").arg(i).arg(toHexLog(data.at(0))).arg(toHexLog(CRC)));
			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------
bool SSPCashAcceptor::performBaudRateChanging(bool aUp)
{
	if (mFirmware && mModelData.baudrateFirmware && (mFirmware < mModelData.baudrateFirmware))
	{
		toLog(LogLevel::Warning, mDeviceName + QString(": Cannot change baud rate in cash acceptor due to it is need %2 firmware min").arg(mModelData.baudrateFirmware));
		return true;
	}
/*
	QString portData = mIOPort->getDeviceConfiguration()[CHardwareSDK::DeviceData].toMap()[CDeviceData::Ports::Mine].toString();

	if (portData.contains(CSSP::HeadConnectionId))
	{
		toLog(LogLevel::Normal, mDeviceName + ": It is no need to change baud rate due to there is head connection");
		//return true;
	}
*/
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
	commandData.append(CSSP::UpdatingFirmware::ContinuousBaudrate);

	if (!processCommand(CSSP::Commands::SetBaudrate, commandData))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to set %1 baud rate in cash acceptor").arg(baudRate));
		return false;
	}

	SleepHelper::msleep(CSSP::UpdatingFirmware::Pause::ChangingBaudrate);

	portParameters[EParameters::BaudRate] = baudRate;

	if (!mIOPort->setParameters(portParameters))
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Failed to set %1 baud rate in port").arg(baudRate));
		return false;
	}

	toLog(LogLevel::Normal, mDeviceName + QString(": Baud rate has changed to %2.").arg(baudRate));

	return reset(true);
}

//--------------------------------------------------------------------------------
