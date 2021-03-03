/* @file Принтер Citizen CPP-8001. */

#pragma once

#include "CitizenBase.h"

//--------------------------------------------------------------------------------
class CitizenCPP8001 : public CitizenBase<TSerialPOSPrinter>
{
	SET_SUBSERIES("CitizenCPP8001")

public:
	CitizenCPP8001();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);
};

//--------------------------------------------------------------------------------
CitizenCPP8001::CitizenCPP8001()
{
	using namespace SDK::Driver::IOPort::COM;

	// параметры порта
	mPortParameters.insert(EParameters::BaudRate, POSPrinters::TSerialDevicePortParameter()
		<< EBaudRate::BR38400
		<< EBaudRate::BR19200
		<< EBaudRate::BR4800
		<< EBaudRate::BR9600);

	// статусы ошибок
	mParameters.errors.clear();

	mParameters.errors[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);

	mParameters.errors[2][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
	mParameters.errors[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
	mParameters.errors[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);

	mParameters.errors[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
	mParameters.errors[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

	mParameters.errors[4][1].insert('\x0C', PrinterStatusCode::Warning::PaperNearEnd);
	mParameters.errors[4][1].insert('\x60', PrinterStatusCode::Error::PaperEnd);

	// параметры моделей
	mDeviceName = "Citizen CPP-8001";
	setModelID('\x20');

	// модели
	mModelData.data().clear();
	mModelData.add(mModelID, true, mDeviceName);

	setConfigParameter(CHardware::Printer::FeedingAmount, 6);
}

//--------------------------------------------------------------------------------
void CitizenCPP8001::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	POSPrinter::setDeviceConfiguration(aConfiguration);

	int lineSpacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();

	int feeding = 6;
	     if (lineSpacing >= 75) feeding = 3;
	else if (lineSpacing >= 60) feeding = 4;
	else if (lineSpacing >= 50) feeding = 5;

	setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
}

//--------------------------------------------------------------------------------
