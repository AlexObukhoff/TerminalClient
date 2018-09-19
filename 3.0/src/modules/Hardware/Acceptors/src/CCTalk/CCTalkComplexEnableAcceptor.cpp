/* @file Устройство приема денег на протоколе ccTalk с 2-ступенчатой схемой включения на прием денег. */

// Modules
#include "Hardware/CoinAcceptors/CCTalkCoinAcceptorBase.h"

// Project
#include "CCTalkComplexEnableAcceptor.h"
#include "CCTalkAcceptorConstants.h"
#include "CCTalkModelData.h"

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
template class CCTalkComplexEnableAcceptor<CCTalkCoinAcceptorBase>;

//---------------------------------------------------------------------------
template <class T>
bool CCTalkComplexEnableAcceptor<T>::applyParTable()
{
	QByteArray commandData = getParTableData() + CCCTalk::DefaultSorterMask;

	return processCommand(CCCTalk::Command::ModifyInhibitsAndRegesters, commandData + commandData);
}

//--------------------------------------------------------------------------------
template <class T>
void CCTalkComplexEnableAcceptor<T>::postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	if (aOldStatusCollection.contains(BillAcceptorStatusCode::Warning::Cheated) &&
	   !aNewStatusCollection.contains(BillAcceptorStatusCode::Warning::Cheated) && getConfigParameter(CHardware::CashAcceptor::Enabled).toBool())
	{
		reset(true);
		bool enabled = enableMoneyAcceptingMode(true);
		setConfigParameter(CHardware::CashAcceptor::Enabled, enabled);
	}

	T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
