/* @file Монетоприемник на протоколе ccTalk с 2-ступенчатой схемой включения на прием денег. */

#pragma once

// Modules
#include "Hardware/Acceptors/CCTalkComplexEnableAcceptor.h"

// Project
#include "CCTalkCoinAcceptorBase.h"

//--------------------------------------------------------------------------------
typedef CCTalkComplexEnableAcceptor<CCTalkCoinAcceptorBase> TCCTalkComplexEnableAcceptor;

class CCTalkComplexEnableCoinAcceptor : public TCCTalkComplexEnableAcceptor
{
public:
	CCTalkComplexEnableCoinAcceptor()
	{
		mModels = getModelList();
	}

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList()
	{
		return CCCTalk::CoinAcceptor::CModelData().getModels(true);
	}
};

//--------------------------------------------------------------------------------
