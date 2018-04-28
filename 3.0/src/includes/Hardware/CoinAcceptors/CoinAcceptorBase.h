/* @file Базовый класс монетоприемников. */

#pragma once

#include "Hardware/CashAcceptors/PortCashAcceptor.h"

//--------------------------------------------------------------------------------
namespace CCoinAcceptor
{
	/// Таймаут отсылки сигнала об отключении монетника, [мс].
	const int DisablingTimeout = 600;
}

//--------------------------------------------------------------------------------
class CoinAcceptorBase : public TSerialCashAcceptor
{
	SET_DEVICE_TYPE(CoinAcceptor)

public:
	CoinAcceptorBase()
	{
		//TODO: надо для всех монетников
		setConfigParameter(CHardware::CashAcceptor::DisablingTimeout, CCoinAcceptor::DisablingTimeout);
		setConfigParameter(CHardware::CashAcceptor::StackedFilter, true);
	}

	/// Принять купюру.
	virtual bool stack()
	{
		// у монетоприемника нет стека
		return true;
	}

	/// Вернуть купюру. Правильный термин - return (ключевое слово).
	virtual bool reject()
	{
		// у монетоприемника нет режекта
		return true;
	}
};

//--------------------------------------------------------------------------------
