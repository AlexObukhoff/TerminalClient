/* @file Принтер Custom TG2480H. */

#pragma once

#include "CustomPrinters.h"

//--------------------------------------------------------------------------------
class CustomTG2480H : public CustomPrinter
{
	SET_SUBSERIES("CustomTG2480H")

public:
	CustomTG2480H()
	{
		auto it = std::find_if(mModelData.data().begin(), mModelData.data().end(), [&] (const POSPrinters::SModelData & aData) -> bool
			{ return aData.name == CCustomPrinter::Models::TG2480; });

		POSPrinters::SParameters parameters = it->parameters;
		parameters.errors->data()[20][4].insert('\x08', PrinterStatusCode::OK::MotorMotion);
		parameters.errors->data()[20][5].insert('\x02', PrinterStatusCode::Error::Presenter);

		mDeviceName = CCustomPrinter::Models::TG2480H;
		mModelID = '\xA8';

		mModelData.data().clear();
		mModelData.add(mModelID, true,  CCustomPrinter::Models::TG2480H, parameters);
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
