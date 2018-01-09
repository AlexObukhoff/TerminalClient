/* @file Ресайклер на протоколе CCNet. */

#pragma once

#include "CCNetCashAcceptorBase.h"

//--------------------------------------------------------------------------------
namespace CCCNetRecycler
{
	/// Выход из initilaize-а.
	const int ExitInitializeTimeout = 30 * 1000;
}

//--------------------------------------------------------------------------------
class CCNetRecycler : public CCNetCashAcceptorBase
{
	//TODO: сделать отдельный тип устройств при реализации диспенсерного функционала
	SET_SUBSERIES("Recycler")

public:
	CCNetRecycler();

protected:
	/// Локальный сброс.
	virtual bool processReset();
};

//--------------------------------------------------------------------------------
