/* @file Купюроприемник Cashcode GX на протоколе CCNet. */

#include "CCNetCashcodeGX.h"
#include "CCNetCashAcceptorConstants.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//---------------------------------------------------------------------------
CCNetCashcodeGX::CCNetCashcodeGX()
{
	// данные устройства
	mDeviceName = CCCNet::Models::CashcodeGX;
	mSupportedModels = QStringList() << mDeviceName;

	setConfigParameter(CHardware::CashAcceptor::InitializeTimeout, CCCNetCashcodeGX::ExitInitializeTimeout);
}

//--------------------------------------------------------------------------------
bool CCNetCashcodeGX::checkConnectionAbility()
{
	if (!CCNetCashAcceptorBase::checkConnectionAbility())
	{
		return false;
	}

	TPortParameters portParameters;
	mIOPort->getParameters(portParameters);
	ERTSControl::Enum RTS = (mIOPort->getType() == EPortTypes::COM) ? ERTSControl::Disable : ERTSControl::Enable;

	portParameters[EParameters::RTS] = RTS;

	mPortParameters[EParameters::RTS].clear();
	mPortParameters[EParameters::RTS].append(RTS);

	return mIOPort->setParameters(portParameters);
}

//---------------------------------------------------------------------------------
TResult CCNetCashcodeGX::performCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	if (mIOPort->getType() == SDK::Driver::EPortTypes::VirtualCOM)
	{
		QVariantMap configuration;
		configuration.insert(CHardware::Port::COM::WaitResult, true);
		//mIOPort->setDeviceConfiguration(configuration);
	}

	return CCNetCashAcceptorBase::performCommand(aCommand, aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
bool CCNetCashcodeGX::processReset()
{
	bool result = processCommand(CCCNet::Commands::Reset);

	mIOPort->close();
	SleepHelper::msleep(CCCNetCashcodeGX::ResetPause);
	mIOPort->open();

	bool wait = waitNotBusyPowerUp();

	return (result && wait) || !mForceWaitResetCompleting;
}

//--------------------------------------------------------------------------------
bool CCNetCashcodeGX::processUpdating(const QByteArray & aBuffer, int aSectionSize)
{
	int sections = int(std::ceil(double(aBuffer.size()) / aSectionSize));
	toLog(LogLevel::Normal, mDeviceName + QString(": section size for updating the firmware = %1, buffer size = %2, amount of sections = %3")
		.arg(aSectionSize).arg(aBuffer.size()).arg(sections));

	int repeat = 0;
	bool result = true;

	for (int i = 0; i < sections; ++i)
	{
		uint address = i * aSectionSize;
		QByteArray buffer = aBuffer.mid(address, aSectionSize);
		buffer += QByteArray(aSectionSize - buffer.size(), ASCII::NUL);

		if (!processBlockUpdating(qToBigEndian(address) >> 8, buffer, repeat, i))
		{
			result = false;

			break;
		}
	}

	SleepHelper::msleep(CCCNet::ExitUpdatingPause);

	return result;
}

//--------------------------------------------------------------------------------
bool CCNetCashcodeGX::canChangeBaudrate()
{
	if (mIOPort->getType() == EPortTypes::COM)
	{
		return true;
	}

	TPortParameters portParameters;
	mIOPort->getParameters(portParameters);

	return portParameters[EParameters::BaudRate] == EBaudRate::BR9600;
}

//--------------------------------------------------------------------------------
bool CCNetCashcodeGX::performBaudRateChanging(const TPortParameters & aPortParameters)
{
	int baudRate = aPortParameters[EParameters::BaudRate];
	QString hexBaudRate = QString("%1").arg(qToBigEndian(uint(baudRate)) >> 8, 6, 16, QChar(ASCII::Zero));

	return processCommand(CCCNet::Commands::UpdatingFirmware::SetBaudRate, ProtocolUtils::getBufferFromString(hexBaudRate)) &&
		mIOPort->setParameters(aPortParameters);
}

//--------------------------------------------------------------------------------
