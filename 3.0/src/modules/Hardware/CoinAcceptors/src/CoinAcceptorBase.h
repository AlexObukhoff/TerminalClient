/* @file Базовый класс монетоприемников. */

#pragma once

#include "Hardware/CashAcceptors/SerialCashAcceptor.h"

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
	CoinAcceptorBase();

	/// Принять купюру.
	virtual bool stack();

	/// Вернуть купюру. Правильный термин - return (ключевое слово).
	virtual bool reject();
};

//--------------------------------------------------------------------------------
