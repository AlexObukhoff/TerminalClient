/* @file Системные принтеры без автопоиска. */

#pragma once

#include "SystemPrinter.h"

//--------------------------------------------------------------------------------
class SunphorPOS58IV : public SystemPrinter
{
	SET_SERIES("Sunphor")
	SET_SUBSERIES("POS58IV")

public:
	SunphorPOS58IV()
	{
		mDeviceName = "Sunphor Printer";
		mLineSize = 30;
		mSideMargin = 4.0;
		mAutoDetectable = false;
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
		mSideMargin = 3.0;
		mAutoDetectable = false;
	}
};

//--------------------------------------------------------------------------------
