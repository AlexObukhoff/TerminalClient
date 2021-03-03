/* @file Системные принтеры. */

#pragma once

#include "SystemPrinter.h"
#include "Sam4sModelData.h"

//--------------------------------------------------------------------------------
class SunphorPOS58IV : public SystemPrinter
{
	SET_SERIES("Sunphor")
	SET_SUBSERIES("POS58IV")

public:
	SunphorPOS58IV()
	{
		mDeviceName = "Sunphor POS58IV";
		mIdName = "Sunphor";
		mLineSize = 30;
		mSideMargin = 4.0;
	}
};

//--------------------------------------------------------------------------------
class ICTGP83 : public SystemPrinter
{
	SET_SERIES("ICT")
	SET_SUBSERIES("GP83")

public:
	ICTGP83()
	{
		mDeviceName = "ICT GP83";
		mIdName = "ICT";
		mSideMargin = 3.0;
	}
};

//--------------------------------------------------------------------------------
