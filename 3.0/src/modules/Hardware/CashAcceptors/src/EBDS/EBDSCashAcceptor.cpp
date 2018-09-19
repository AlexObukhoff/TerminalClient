/* @file Купюроприемник на протоколе EBDS. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>

// Project
#include "EBDSCashAcceptor.h"
#include "EBDSCashAcceptorConstants.h"
#include "EBDSModelData.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
EBDSCashAcceptor::EBDSCashAcceptor()
{
	// параметры порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
	mPortParameters[EParameters::Parity].append(EParity::Even);

	mPortParameters[EParameters::ByteSize].clear();
	mPortParameters[EParameters::ByteSize].append(7);

	// данные устройства
	mDeviceName = "EBDS cash acceptor";
	mStackerNearFull = false;
	mResetWaiting = EResetWaiting::Available;
	mEnabled = false;

	// параметры протокола
	mDeviceCodeSpecification = PDeviceCodeSpecification(new CEBDS::DeviceCodeSpecification);
}

//--------------------------------------------------------------------------------
QStringList EBDSCashAcceptor::getModelList()
{
	QSet<QString> result;

	foreach (SBaseModelData aData, CEBDS::ModelData().data().values())
	{
		result << aData.name;
	}

	return result.toList();
}

//---------------------------------------------------------------------------------
bool EBDSCashAcceptor::checkStatus(QByteArray & aAnswer)
{
	return poll(0, &aAnswer);
}

//---------------------------------------------------------------------------------
TResult EBDSCashAcceptor::execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	MutexLocker locker(&mExternalMutex);

	mProtocol.setPort(mIOPort);
	mProtocol.setLog(mLog);

	QByteArray answer;
	TResult result = mProtocol.processCommand(aCommand + aCommandData, answer, aCommand != CEBDS::Commands::Reset);

	if (!result)
	{
		return result;
	}

	bool isValidStandardMessage    =  aCommand.startsWith(CEBDS::Commands::Host2Validator) &&  answer.startsWith(CEBDS::Commands::Validator2Host);
	bool isValidNonStandardMessage = !aCommand.startsWith(CEBDS::Commands::Host2Validator) && !answer.startsWith(CEBDS::Commands::Validator2Host);
	bool isValidSpecialMessage     = (aCommand.startsWith(CEBDS::Commands::SetInhibits)    &&  answer.startsWith(CEBDS::Commands::Validator2Host)) ||
	                                 (aCommand.startsWith(CEBDS::Commands::Host2Validator) &&  answer.startsWith(CEBDS::Commands::GetPar));
	bool isValidControlMessage     =  aCommand.startsWith(CEBDS::Commands::Control)        &&  answer.startsWith(CEBDS::Commands::Control);

	if (!isValidStandardMessage && !isValidSpecialMessage && !isValidNonStandardMessage && !isValidControlMessage)
	{
		toLog(LogLevel::Error, mDeviceName + ": Invalid command type of the message.");
		return CommandResult::Protocol;
	}

	if (aAnswer)
	{
		int index = answer.startsWith(CEBDS::Commands::Extended) ? 2 : 1;
		*aAnswer = answer.mid(index);
	}

	return CommandResult::OK;
}

//---------------------------------------------------------------------------------
TResult EBDSCashAcceptor::poll(char aAction, QByteArray * aAnswer)
{
	QByteArray commandData;
	commandData.append(mEnabled ? ASCII::Full : ASCII::NUL);
	commandData.append(CEBDS::Byte1 | aAction);
	commandData.append(CEBDS::Byte2);

	return processCommand(CEBDS::Commands::Host2Validator, commandData, aAnswer);
}

//--------------------------------------------------------------------------------
bool EBDSCashAcceptor::processReset()
{
	//TODO: ответа не будет, поллить минимум 5 секунд
	return processCommand(CEBDS::Commands::Reset);
}

//--------------------------------------------------------------------------------
bool EBDSCashAcceptor::isConnected()
{
	QByteArray answer;

	if (!poll(0, &answer))
	{
		return false;
	}

	char modelKey = answer[4];
	double revision = answer[5] / 10.0;
	CEBDS::ModelData modelData;
	bool advanced = false;

	if (processCommand(CEBDS::Commands::GetType, &answer))
	{
		advanced = answer.contains(CEBDS::AdvancedModelTag) && modelData.data().contains(CEBDS::TModelData(modelKey, true));
	}

	SBaseModelData data = modelData[CEBDS::TModelData(modelKey, advanced)];
	mDeviceName = data.name;
	mVerified = data.verified;

	setDeviceParameter(CDeviceData::ModelKey, ProtocolUtils::toHexLog(modelKey));
	setDeviceParameter(CDeviceData::Revision, revision);

	return true;
}

//--------------------------------------------------------------------------------
void EBDSCashAcceptor::processDeviceData()
{
	QByteArray answer;
	auto getData = [&] (const QByteArray & aCommand) -> bool { bool result = processCommand(aCommand, &answer); answer.replace(ASCII::NUL, "").replace(ASCII::DEL, ""); return result; };

	if (getData(CEBDS::Commands::GetType))
	{
		setDeviceParameter(CDeviceData::Type, answer);
	}

	if (getData(CEBDS::Commands::GetSerialNumber))
	{
		setDeviceParameter(CDeviceData::SerialNumber, answer);
	}

	auto checkSoftData = [&] (const QByteArray & aCommand, const QString & aMainKey) { removeDeviceParameter(aMainKey); if (getData(aCommand)) {
		setDeviceParameter(CDeviceData::ProjectNumber, answer.left(5).toInt(), aMainKey);
		setDeviceParameter(CDeviceData::Version, answer.right(3).toDouble() / 100, aMainKey); } };

	checkSoftData(CEBDS::Commands::GetAppSoftVersion,  CDeviceData::Firmware);
	checkSoftData(CEBDS::Commands::GetBootSoftVersion, CDeviceData::BootVersion);
	checkSoftData(CEBDS::Commands::GetVariantVersion,  CDeviceData::CashAcceptors::BillSet);

	if (getData(CEBDS::Commands::GetVariantName))
	{
		setDeviceParameter(CDeviceData::Type, answer, CDeviceData::CashAcceptors::BillSet);
	}
}

//---------------------------------------------------------------------------
bool EBDSCashAcceptor::stack()
{
	if (!checkConnectionAbility() || (mInitialized != ERequestStatus::Success) || mCheckDisable)
	{
		return false;
	}

	bool result = poll(CEBDS::Stack);
	reenableMoneyAcceptingMode();

	return result;
}

//---------------------------------------------------------------------------
bool EBDSCashAcceptor::reject()
{
	if (!checkConnectionAbility() || (mInitialized == ERequestStatus::Fail))
	{
		return false;
	}

	bool result = poll(CEBDS::Return);
	reenableMoneyAcceptingMode();

	return result;
}

//---------------------------------------------------------------------------
bool EBDSCashAcceptor::applyParTable()
{
	QByteArray commandData;
	commandData.append(mEnabled ? ASCII::Full : ASCII::NUL);
	commandData.append(CEBDS::Byte1);
	commandData.append(CEBDS::Byte2);

	commandData += QByteArray(8, ASCII::NUL);

	for (auto it = mEscrowParTable.data().begin(); it != mEscrowParTable.data().end(); ++it)
	{
		if (it->enabled && !it->inhibit)
		{
			int id = it.key() - 1;
			int index = 3 + id / 7;
			commandData[index] = commandData[index] | (1 << (id % 7));
		}
	}

	if (!processCommand(CEBDS::Commands::SetInhibits, commandData))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to set nominal inhibits for receiving money");
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
bool EBDSCashAcceptor::enableMoneyAcceptingMode(bool aEnabled)
{
	mEnabled = aEnabled;
	setConfigParameter(CHardware::CashAcceptor::Enabled, aEnabled);

	return true;
}

//--------------------------------------------------------------------------------
bool EBDSCashAcceptor::loadParTable()
{
	QByteArray commandData;
	commandData.append(ASCII::NUL);
	commandData.append(CEBDS::Byte1);
	commandData.append(CEBDS::Byte2);

	QByteArray answer;
	bool result = true;

	for (int i = 0; i < CEBDS::NominalCount; ++i)
	{
		commandData[3] = uchar(i) + 1;

		if (!processCommand(CEBDS::Commands::GetPar, commandData, &answer))
		{
			toLog(LogLevel::Error, mDeviceName + QString(": Failed to get data for nominal %1").arg(i + 1));
			result = false;
		}
		else
		{
			MutexLocker locker(&mResourceMutex);

			mEscrowParTable.data().insert(i + 1, getPar(answer));
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
SPar EBDSCashAcceptor::getPar(const QByteArray & aData)
{
	if (aData.size() < CEBDS::NominalSize)
	{
		toLog(LogLevel::Error, mDeviceName + QString(": Too small answer size = %1 for nominal, need %2 minimum").arg(aData.size()).arg(CEBDS::NominalSize));
		return SPar();
	}

	int nominal = aData.mid(10, 3).toInt() * int(qPow(10, aData.mid(13, 3).toDouble()));
	QString currency = QString(aData.mid(7, 3));

	SPar par(nominal, CurrencyCodes[currency]);
	par.currency = currency;

	return par;
}

//--------------------------------------------------------------------------------
bool EBDSCashAcceptor::setLastPar(const QByteArray & aAnswer)
{
	SDK::Driver::SPar par = getPar(aAnswer);
	mEscrowPars  = TPars() << par;

	return mEscrowParTable.data().values().contains(par) && (par.nominal) && (par.currencyId != Currency::NoCurrency);
}

//--------------------------------------------------------------------------------
void EBDSCashAcceptor::cleanSpecificStatusCodes(TStatusCodes & aStatusCodes)
{
	mStackerNearFull = mStackerNearFull ||
		(aStatusCodes.contains(BillAcceptorStatusCode::BillOperation::Stacked) &&
		 aStatusCodes.contains(BillAcceptorStatusCode::Warning::Cheated));

	if (mStackerNearFull)
	{
		aStatusCodes.remove(BillAcceptorStatusCode::Warning::Cheated);
		aStatusCodes.insert(BillAcceptorStatusCode::Warning::StackerNearFull);
	}
}

//--------------------------------------------------------------------------------
