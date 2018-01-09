/* @file Монетоприемник на протоколе ccTalk с 2-ступенчатой схемой включения на прием денег. */

#include "ComplexEnable.h"
#include "CCTalkCoinAcceptorConstants.h"
#include "CCTalkModelData.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
QStringList ComplexEnableCCTalkCoinAcceptor::getModelList()
{
	return CCCTalk::getModels(true);
}

//--------------------------------------------------------------------------------
ComplexEnableCCTalkCoinAcceptor::ComplexEnableCCTalkCoinAcceptor()
{
	mModels = getModelList();
}

//---------------------------------------------------------------------------
bool ComplexEnableCCTalkCoinAcceptor::applyParTable()
{
	QByteArray commandData = getParTableData() + CCCTalk::DefaultSorterMask;

	return processCommand(CCCTalk::Command::ModifyInhibitsAndRegesters, commandData + commandData);
}

//--------------------------------------------------------------------------------
void ComplexEnableCCTalkCoinAcceptor::postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	if (aOldStatusCollection.contains(BillAcceptorStatusCode::Warning::Cheated) &&
	   !aNewStatusCollection.contains(BillAcceptorStatusCode::Warning::Cheated) && getConfigParameter(CHardware::CashAcceptor::Enabled).toBool())
	{
		reset(true);
		bool enabled = enableMoneyAcceptingMode(true);
		setConfigParameter(CHardware::CashAcceptor::Enabled, enabled);
	}

	CCTalkCoinAcceptorBase::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//--------------------------------------------------------------------------------
