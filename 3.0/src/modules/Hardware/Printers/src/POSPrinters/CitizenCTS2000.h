/* @file Принтер Citizen CTS-2000. */

#pragma once

#include "Hardware/Printers/POSPrinter.h"

//--------------------------------------------------------------------------------
class CitizenCTS2000 : public POSPrinter
{
	SET_SUBSERIES("CitizenCTS2000")

public:
	CitizenCTS2000();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);
};

//--------------------------------------------------------------------------------
CitizenCTS2000::CitizenCTS2000()
{
	POSPrinters::SParameters parameters(mModelData.getDefault().parameters);

	// статусы ошибок
	parameters.errors->data().clear();

	parameters.errors->data()[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);

	parameters.errors->data()[2][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
	parameters.errors->data()[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
	parameters.errors->data()[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);

	parameters.errors->data()[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
	parameters.errors->data()[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

	parameters.errors->data()[4][1].insert('\x0C', PrinterStatusCode::Warning::PaperNearEnd);
	parameters.errors->data()[4][1].insert('\x60', PrinterStatusCode::Error::PaperEnd);

	// теги
	parameters.tagEngine->appendCommon(Tags::Type::DoubleWidth,  "\x1B\x21", "\x20");
	parameters.tagEngine->appendCommon(Tags::Type::DoubleHeight, "\x1B\x21", "\x10");

	// параметры моделей
	setConfigParameter(CHardware::Printer::FeedingAmount, 5);
	mDeviceName = "Citizen CT-S2000";
	mModelID = '\x51';

	// модели
	mModelData.data().clear();
	mModelData.add(mModelID, true, mDeviceName, parameters);
	mPortParameters = parameters.portSettings->data();
}

//--------------------------------------------------------------------------------
void CitizenCTS2000::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	POSPrinter::setDeviceConfiguration(aConfiguration);

	int lineSpacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();

	int feeding = 5;
	     if (lineSpacing >= 75) feeding = 2;
	else if (lineSpacing >= 55) feeding = 3;
	else if (lineSpacing >= 45) feeding = 4;

	setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
}

//--------------------------------------------------------------------------------
