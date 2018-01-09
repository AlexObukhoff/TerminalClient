/* @file Ярус-01К. */

#pragma once

#include "ShtrihRetractorFR.h"

//--------------------------------------------------------------------------------
class Yarus01K : public ShtrihRetractorFR
{
	SET_SUBSERIES("Yarus01K")

public:
	Yarus01K()
	{
		mDeviceName = CShtrihFR::Models::CData()[CShtrihFR::Models::ID::Yarus01K].name;
		mSupportedModels = QStringList() << mDeviceName;
	}
};

//--------------------------------------------------------------------------------
