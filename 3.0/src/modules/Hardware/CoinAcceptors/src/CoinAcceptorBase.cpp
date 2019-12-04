/* @file Базовый класс монетоприемников. */

#pragma once

#include "CoinAcceptorBase.h"

//--------------------------------------------------------------------------------
CoinAcceptorBase::CoinAcceptorBase()
{
	mDeviceType = CHardware::Types::CoinAcceptor;
	setConfigParameter(CHardware::CashAcceptor::DisablingTimeout, CCoinAcceptor::DisablingTimeout);
	setConfigParameter(CHardware::CashAcceptor::StackedFilter, true);
}

//--------------------------------------------------------------------------------
bool CoinAcceptorBase::stack()
{
	// у монетоприемника нет стека
	return true;
}

//--------------------------------------------------------------------------------
bool CoinAcceptorBase::reject()
{
	// у монетоприемника нет режекта
	return true;
}

//--------------------------------------------------------------------------------
