/* @file Сторожевой таймер STOD. */

#pragma once

#include "OSMP.h"

//--------------------------------------------------------------------------------
class STOD: public OSMP
{
	SET_SERIES("STOD")

public:
	STOD()
	{
		mDeviceName = "STOD";

		mData[EOSMPCommandId::IdentificationData] = "STODSIM";

		mData[EOSMPCommandId::Identification] = "OSP\x09";
		mData[EOSMPCommandId::RebootPC] = "OSTR";
	}
};

//--------------------------------------------------------------------------------
