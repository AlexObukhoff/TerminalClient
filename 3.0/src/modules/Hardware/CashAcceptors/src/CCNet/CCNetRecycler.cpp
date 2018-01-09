/* @file Ресайклер на протоколе CCNet. */

#include "CCNetRecycler.h"
#include "CCNetCashAcceptorConstants.h"

//---------------------------------------------------------------------------
CCNetRecycler::CCNetRecycler()
{
	// данные устройства
	mDeviceName = CCCNet::Models::CashcodeG200;
	mSupportedModels = QStringList() << mDeviceName;
	mResetWaiting = EResetWaiting::Full;

	setConfigParameter(CHardware::CashAcceptor::InitializeTimeout, CCCNetRecycler::ExitInitializeTimeout);

	// параметры протокола
	mProtocol.setAddress(CCCNet::Addresses::BillToBill);
}

//--------------------------------------------------------------------------------
bool CCNetRecycler::processReset()
{
	bool result = processCommand(CCCNet::Commands::Reset);

	if (!waitNotBusyPowerUp())
	{
		result = processCommand(CCCNet::Commands::Reset);
		waitNotBusyPowerUp();
	}

	return result || !mForceWaitResetCompleting;
}


//--------------------------------------------------------------------------------
