/* @file Базовый принтер Citizen. */

#pragma once

#include "Hardware/Printers/POSPrinter.h"

//--------------------------------------------------------------------------------
template <class T>
class CitizenBase : public T
{
public:
	CitizenBase()
	{
		mRussianCodePage = '\x07';

		// теги
		POSPrinters::SModelData modelData = mModelData.getDefault();
		modelData.parameters.tagEngine->appendCommon(Tags::Type::DoubleWidth,  "\x1B\x21", "\x20");
		modelData.parameters.tagEngine->appendCommon(Tags::Type::DoubleHeight, "\x1B\x21", "\x10");

		mModelData.setDefault(modelData);
	}
};

//--------------------------------------------------------------------------------
