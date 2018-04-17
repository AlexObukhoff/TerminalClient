/* @file Структура для функционала ожидания состояния. */

#pragma once

//--------------------------------------------------------------------------------
struct SWaitingData
{
	int pollingInterval;
	int timeout;
	bool pollingSensible;

	SWaitingData(): pollingInterval(0), timeout(0), pollingSensible(false) {}
	SWaitingData(int aPollingInterval, int aTimeout, bool aPollingSensible = false):
		pollingInterval(aPollingInterval), timeout(aTimeout), pollingSensible(aPollingSensible) {}
};

//--------------------------------------------------------------------------------
