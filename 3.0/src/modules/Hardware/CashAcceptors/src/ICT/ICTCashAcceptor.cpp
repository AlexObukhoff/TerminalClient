/* @file Купюроприемник на протоколе ICT. */

// Project
#include "ICTCashAcceptor.h"
#include "ICTCashAcceptorConstants.h"
#include "ICTModelData.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
ICTCashAcceptor::ICTCashAcceptor()
{
	// параметры порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);
	mPortParameters[EParameters::Parity].append(EParity::Even);

	mIOMessageLogging = ELoggingType::ReadWrite;

	// данные устройства
	mDeviceName = "ICT cash acceptor";

	// параметры протокола
	mDeviceCodeSpecification = PDeviceCodeSpecification(new CICTBase::DeviceCodeSpecification);
}

//--------------------------------------------------------------------------------
QStringList ICTCashAcceptor::getModelList()
{
	return CICT::ModelData().data().keys();
}

//--------------------------------------------------------------------------------
bool ICTCashAcceptor::processReset()
{
	if (mOldFirmware)
	{
		toLog(LogLevel::Warning, mDeviceName + ": protocol version ICT002, Reset is out of keeping");
		return true;
	}

	QByteArray answer;

	if (!mIOPort->write(QByteArray(1, CICTBase::Commands::Reset)))
	{
		return false;
	}

	SleepHelper::msleep(CICTBase::ResetTimeout);

	if (!mIOPort->read(answer, 100))
	{
		return false;
	}

	if (answer.isEmpty())
	{
		toLog(LogLevel::Warning, mDeviceName + ": There is no answer from peripheral device. Perhaps, this is protocol ICT002");
		mOldFirmware = true;

		return true;
	}

	if (!answer.contains(CICTBase::States::PowerUp))
	{
		toLog(LogLevel::Error, mDeviceName + ": Invalid response");
		return false;
	}

	return answerToReset();
}

//---------------------------------------------------------------------------------
bool ICTCashAcceptor::checkStatus(QByteArray & aAnswer)
{
	if (!mIOPort->read(aAnswer, 100))
	{
		return false;
	}

	auto poll = [&] () -> bool { return mIOPort->write(QByteArray(1, CICTBase::Commands::Poll)) && mIOPort->read(aAnswer, 100) && !aAnswer.isEmpty(); };

	if (aAnswer.isEmpty())
	{
		if (!poll())
		{
			return false;
		}

		if (aAnswer[0] == CICTBase::States::Disabled)
		{
			if (!enableMoneyAcceptingMode(true) || !poll())
			{
				return false;
			}

			if ((aAnswer.size() > 1) || (aAnswer[0] != CICTBase::States::Idling))
			{
				aAnswer = aAnswer.replace(CICTBase::States::Idling, "");
			}

			enableMoneyAcceptingMode(false);

			if (!poll())
			{
				return false;
			}
		}
	}

	if (aAnswer.contains(CICTBase::States::PowerUp))
	{
		answerToReset();
	}

	// Если купюрник вышел из ошибки, то чистим буфер ответа, спрашиваем еще раз статус и выходим
	if (aAnswer.contains(CICTBase::States::ErrorExlusion) && !poll())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to poll after error exlusion");
		return false;
	}

	int escrowIndex = aAnswer.indexOf(CICTBase::States::Escrow);

	if (escrowIndex != -1)
	{
		mEscrowPosition = escrowIndex + 1;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool ICTCashAcceptor::isConnected()
{
	if (!mIOPort->write(CICTBase::Commands::Identification))
	{
		return false;
	}

	SleepHelper::msleep(CICTBase::ResetTimeout);
	QByteArray answer;

	if (!mIOPort->read(answer, 100))
	{
		return false;
	}

	if (answer.size() < 3)
	{
		toLog(LogLevel::Error, mDeviceName + ": Perhaps unknown device trying to impersonate any ICT device");
		return false;
	}

	CICTBase::DeviceCodeSpecification * specification = mDeviceCodeSpecification.dynamicCast<CICTBase::DeviceCodeSpecification>().data();

	if ((answer.right(3) != CICTBase::Answers::Identification) &&
		!((answer.size() > 2) && (answer[0] == answer[1]) && (answer[0] == answer[2]) && specification->contains(answer[0])))
	{
		return false;
	}

	QString configModelName;

	if (!isAutoDetecting())
	{
		configModelName = getConfigParameter(CHardwareSDK::ModelName).toString();
	}

	mDeviceName = configModelName.isEmpty() ? "ICT U70" : configModelName;
	mVerified = CICT::ModelData()[mDeviceName];

	return true;
}

//--------------------------------------------------------------------------------
bool ICTCashAcceptor::answerToReset()
{
	if (!mIOPort->write(QByteArray(1, CICTBase::Commands::ACK)))
	{
		return false;
	}

	SleepHelper::msleep(CICTBase::ResetTimeout);
	enableMoneyAcceptingMode(false);
	SleepHelper::msleep(CICTBase::ResetTimeout);

	return true;
}

//---------------------------------------------------------------------------
bool ICTCashAcceptor::stack()
{
	if (!checkConnectionAbility() || (mInitialized != ERequestStatus::Success) || mCheckDisable)
	{
		return false;
	}

	return mIOPort->write(QByteArray(1, CICTBase::Commands::ACK));
}

//---------------------------------------------------------------------------
bool ICTCashAcceptor::reject()
{
	if (!checkConnectionAbility() || (mInitialized == ERequestStatus::Fail))
	{
		return false;
	}

	return mIOPort->write(QByteArray(1, CICTBase::Commands::NAK));
}

//---------------------------------------------------------------------------
bool ICTCashAcceptor::enableMoneyAcceptingMode(bool aEnabled)
{
	char command = aEnabled ? CICTBase::Commands::Enable : CICTBase::Commands::Disable;

	return mIOPort->write(QByteArray(1, command));
}

//--------------------------------------------------------------------------------
bool ICTCashAcceptor::loadParTable()
{
	mEscrowParTable.add(0x40, SPar(  10, Currency::RUB));
	mEscrowParTable.add(0x41, SPar(  50, Currency::RUB));
	mEscrowParTable.add(0x42, SPar( 100, Currency::RUB));
	mEscrowParTable.add(0x43, SPar( 500, Currency::RUB));
	mEscrowParTable.add(0x44, SPar(1000, Currency::RUB));

	return true;
}

//--------------------------------------------------------------------------------
bool ICTCashAcceptor::isStatusesReplaceable(TStatusCodes & aStatusCodes)
{
	TStatusCodes errors = mStatusCollection.value(EWarningLevel::Error);
	auto check = [&errors, &aStatusCodes] (int statusCode) -> bool { return aStatusCodes.contains(statusCode) && !errors.contains(statusCode); };

	foreach (int statusCode, aStatusCodes)
	{
		if ((mStatusCodesSpecification->value(statusCode).warningLevel == EWarningLevel::Error) && check(statusCode))
		{
			return true;
		}
	}

	return TSerialCashAcceptor::isStatusesReplaceable(aStatusCodes);
}

//--------------------------------------------------------------------------------
void ICTCashAcceptor::postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	if (isPowerReboot() && mOldFirmware)
	{
		TStatusCodes statusCodes;
		auto poll = [&] () -> bool { return getStatus(std::ref(statusCodes)); };

		CICTBase::DeviceCodeSpecification * specification = mDeviceCodeSpecification.dynamicCast<CICTBase::DeviceCodeSpecification>().data();
		auto isPowerUp = [&] () -> bool {return std::find_if(mDeviceCodeBuffers.begin(), mDeviceCodeBuffers.end(), [&] (const QByteArray & aBuffer) -> bool
			{ return specification->isPowerUp(aBuffer); }) != mDeviceCodeBuffers.end(); };

		PollingExpector().wait<bool>(poll, isPowerUp, CICTBase::PowerUpWaiting);
	}

	TSerialCashAcceptor::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}


//--------------------------------------------------------------------------------
