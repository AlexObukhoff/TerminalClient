/* @file Принтер Custom TG2480H. */

#pragma once

#include "CustomPrinters.h"

//--------------------------------------------------------------------------------
template <class T>
class CustomTG2480H : public T
{
	SET_SUBSERIES("CustomTG2480H")

public:
	CustomTG2480H()
	{
		mParameters.errors[20][4].insert('\x08', PrinterStatusCode::OK::MotorMotion);
		mParameters.errors[20][5].insert('\x02', PrinterStatusCode::Error::Presenter);

		mDeviceName = CCustomPrinter::Models::TG2480H;
		setModelID('\xA8');

		mModelData.data().clear();
		mModelData.add(mModelID, true,  CCustomPrinter::Models::TG2480H);
		setConfigParameter(CHardwareSDK::Printer::LineSize, 44);
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
class LibUSBCustomTG2480H: public CustomTG2480H<CustomPrinter<TLibUSBPrinterBase>>
{
public:
	LibUSBCustomTG2480H()
	{
		mDetectingData->set(CUSBVendors::Custom, mDeviceName, 0x01a8);
		setConfigParameter(CHardwareSDK::Printer::LineSize, 44);
	}
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<CustomTG2480H<CustomPrinter<TSerialPrinterBase>>> SerialCustomTG2480H;

//--------------------------------------------------------------------------------
