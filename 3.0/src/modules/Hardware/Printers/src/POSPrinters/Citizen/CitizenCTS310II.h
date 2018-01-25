/* @file Принтер Citizen CT-S310II. */

#pragma once

#include "CitizenBase.h"

//--------------------------------------------------------------------------------
class CitizenCTS310II : public CitizenBase<POSPrinter>
{
	SET_SUBSERIES("CitizenCTS310II")

public:
	CitizenCTS310II();
};

CitizenCTS310II::CitizenCTS310II()
{
	POSPrinters::SParameters parameters(mModelData.getDefault().parameters);

	// статусы ошибок
	parameters.errors->data().clear();

	parameters.errors->data()[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);

	parameters.errors->data()[2][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
	parameters.errors->data()[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
	parameters.errors->data()[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);

	parameters.errors->data()[3][1].insert('\x04', DeviceStatusCode::Error::MechanismPosition);
	parameters.errors->data()[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
	parameters.errors->data()[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

	parameters.errors->data()[4][1].insert('\x0C', PrinterStatusCode::Warning::PaperNearEnd);
	parameters.errors->data()[4][1].insert('\x60', PrinterStatusCode::Error::PaperEnd);

	// параметры моделей
	setConfigParameter(CHardware::Printer::FeedingAmount, 3);
	mDeviceName = "Citizen CT-S310II";
	mModelID = '\x3D';

	// модели
	mModelData.data().clear();
	mModelData.add(mModelID, true, mDeviceName, parameters);
	mPortParameters = parameters.portSettings->data();
}

//--------------------------------------------------------------------------------
