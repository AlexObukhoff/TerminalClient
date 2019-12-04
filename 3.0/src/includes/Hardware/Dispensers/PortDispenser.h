/* @file Диспенсер на порту. */

#pragma once

// Project
#include "Hardware/Dispensers/SerialDispenser.h"

//--------------------------------------------------------------------------------
class PortDispenser : public TSerialDispenser
{
public:
	PortDispenser()
	{
		mIOMessageLogging = ELoggingType::None;
		mMaxBadAnswers = 2;
		mPollingInterval = CDispensers::IdlingPollingInterval;
		mForceNotWaitFirst = true;
	}
};

//--------------------------------------------------------------------------------
