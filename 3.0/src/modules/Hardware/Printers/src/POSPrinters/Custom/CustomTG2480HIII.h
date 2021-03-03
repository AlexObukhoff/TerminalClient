/* @file Принтер Custom TG2480H. */

#pragma once

#include "CustomTG2480H.h"
#include "CustomPrintersIII.h"

//--------------------------------------------------------------------------------
template <class T>
class CustomTG2480HIII : public CustomTG2480H<T>
{
	SET_SUBSERIES("CustomTG2480HIII")

public:
	CustomTG2480HIII()
	{
		mDeviceName = CCustomPrinterIII::Models::TG2480HIII;
		
		mModelData.data().clear();
		mModelData.add("\x02\x44", true,  CCustomPrinterIII::Models::TG2480HIII);
	}
};

//--------------------------------------------------------------------------------
class LibUSBCustomTG2480HIII: public CustomTG2480HIII<CustomPrinterIII<TLibUSBPrinterBase>>
{
public:
	LibUSBCustomTG2480HIII()
	{
		mDetectingData->set(CUSBVendors::Custom, mDeviceName, 0x01a8);
		setConfigParameter(CHardwareSDK::Printer::LineSize, 44);
	}
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<CustomTG2480HIII<CustomPrinterIII<TSerialPrinterBase>>> SerialCustomTG2480HIII;

//--------------------------------------------------------------------------------
