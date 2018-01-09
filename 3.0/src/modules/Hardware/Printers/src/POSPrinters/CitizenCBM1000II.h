/* @file Принтер Citizen CBM-1000II. */

#pragma once

#include "Hardware/Printers/POSPrinter.h"

//--------------------------------------------------------------------------------
class CitizenCBM1000II : public POSPrinter
{
	SET_SUBSERIES("CitizenCBM1000II")

public:
	CitizenCBM1000II();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);
};

//--------------------------------------------------------------------------------
CitizenCBM1000II::CitizenCBM1000II()
{
	using namespace SDK::Driver::IOPort::COM;

	POSPrinters::SParameters parameters(mModelData.getDefault().parameters);

	// параметры порта
	parameters.portSettings->data().insert(EParameters::BaudRate, POSPrinters::TSerialDevicePortParameter()
		<< EBaudRate::BR38400
		<< EBaudRate::BR19200
		<< EBaudRate::BR4800
		<< EBaudRate::BR9600);

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
	mDeviceName = "Citizen CBM-1000II";
	mModelID = '\x30';
	setConfigParameter(CHardware::Printer::FeedingAmount, 3);

	// модели
	mModelData.data().clear();
	mModelData.add(mModelID, true, mDeviceName, parameters);
	mPortParameters = parameters.portSettings->data();

	setConfigParameter(CHardware::Printer::FeedingAmount, 5);
}

//--------------------------------------------------------------------------------
void CitizenCBM1000II::setDeviceConfiguration(const QVariantMap & aConfiguration)
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
