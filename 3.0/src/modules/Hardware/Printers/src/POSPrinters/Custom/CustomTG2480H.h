/* @file Принтер Custom TG2480H. */

#pragma once

#include "CustomPrinters.h"

//--------------------------------------------------------------------------------
template <class T>
class CustomTG2480H : public CustomPrinter<T>
{
	SET_SUBSERIES("CustomTG2480H")

public:
	CustomTG2480H()
	{
		mParameters.errors[20][4].insert('\x08', PrinterStatusCode::OK::MotorMotion);
		mParameters.errors[20][5].insert('\x02', PrinterStatusCode::Error::Presenter);

		mDeviceName = CCustomPrinter::Models::TG2480H;
		mModelID = '\xA8';

		mModelData.data().clear();
		mModelData.add(mModelID, true,  CCustomPrinter::Models::TG2480H);
	}

protected:
	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes)
	{
		if (!CustomPrinter::getStatus(aStatusCodes))
		{
			return false;
		}

		if (aStatusCodes.contains(PrinterStatusCode::Error::Presenter))
		{
			aStatusCodes.remove(PrinterStatusCode::Error::Presenter);

			cut();
			cut();
		}

		return true;
	}
};

//--------------------------------------------------------------------------------
class LibUSBCustomTG2480H: public CustomTG2480H<TLibUSBPrinterBase>
{
public:
	LibUSBCustomTG2480H()
	{
		mDetectingData->set(CUSBVendors::Custom, mDeviceName, 0x01a8);
	}
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<CustomTG2480H<TSerialPrinterBase>> SerialCustomTG2480H;

//--------------------------------------------------------------------------------
