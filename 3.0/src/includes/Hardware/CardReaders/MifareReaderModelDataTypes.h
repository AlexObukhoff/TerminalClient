/* @file Типы данных моделей Mifare-ридеров ACS. */
#pragma once

#include "Hardware/Common/USBDeviceModelData.h"

//------------------------------------------------------------------------------
namespace CMifareReader
{
	struct SModelData: public CUSBDevice::SProductData
	{
		int SAM;
		bool CCID;

		SModelData() : SAM(0), CCID(true) {}
		SModelData(const QString & aModel, int aSAM, bool aCCID, bool aVerified) : CUSBDevice::SProductData(aModel, aVerified), SAM(aSAM), CCID(aCCID) {}
	};
}

//------------------------------------------------------------------------------
