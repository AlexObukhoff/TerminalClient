/* @file ФР PayVKP-80. */

#pragma once

#include "../../Ejector/AtolVKP80BasedFR.h"

//--------------------------------------------------------------------------------
class PayVKP80 : public AtolVKP80BasedFR<AtolSerialFR>
{
	SET_SUBSERIES("PayVKP80K")

public:
	PayVKP80()
	{
		mDeviceName = CAtolFR::Models::PayVKP80K;
		mSupportedModels = QStringList() << mDeviceName;
	}
};

//--------------------------------------------------------------------------------
