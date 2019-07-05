/* @file Базовый принтер Citizen. */

#pragma once

#include "Hardware/Printers/PortPOSPrinters.h"

//--------------------------------------------------------------------------------
template <class T>
class CitizenBase : public T
{
public:
	CitizenBase()
	{
		mRussianCodePage = '\x07';

		// теги
		mParameters = POSPrinters::CommonParameters;
		mParameters.tagEngine.appendCommon(Tags::Type::DoubleWidth,  "\x1B\x21", "\x20");
		mParameters.tagEngine.appendCommon(Tags::Type::DoubleHeight, "\x1B\x21", "\x10");
	}
};

//--------------------------------------------------------------------------------
