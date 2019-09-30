/* @file Принтер Citizen CT-S310II. */

#pragma once

#include "CitizenBase.h"

//--------------------------------------------------------------------------------
class CitizenCTS310II : public CitizenBase<TSerialPOSPrinter>
{
	SET_SUBSERIES("CitizenCTS310II")

public:
	CitizenCTS310II();
};

//--------------------------------------------------------------------------------
CitizenCTS310II::CitizenCTS310II()
{
	// статусы ошибок
	mParameters.errors.clear();

	mParameters.errors[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);

	mParameters.errors[2][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
	mParameters.errors[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
	mParameters.errors[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);

	mParameters.errors[3][1].insert('\x04', DeviceStatusCode::Error::MechanismPosition);
	mParameters.errors[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
	mParameters.errors[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

	mParameters.errors[4][1].insert('\x0C', PrinterStatusCode::Warning::PaperNearEnd);
	mParameters.errors[4][1].insert('\x60', PrinterStatusCode::Error::PaperEnd);

	// параметры моделей
	setConfigParameter(CHardware::Printer::FeedingAmount, 3);
	mDeviceName = "Citizen CT-S310II";
	mModelID = '\x3D';

	// модели
	mModelData.data().clear();
	mModelData.add(mModelID, true, mDeviceName);
}

//--------------------------------------------------------------------------------
