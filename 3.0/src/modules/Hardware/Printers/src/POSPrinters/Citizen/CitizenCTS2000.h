/* @file Принтер Citizen CTS-2000. */

#pragma once

#include "CitizenBase.h"

//--------------------------------------------------------------------------------
template <class T>
class CitizenCTS2000 : public CitizenBase<POSPrinter<T>>
{
	SET_SUBSERIES("CitizenCTS2000")

public:
	CitizenCTS2000();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);
};

//--------------------------------------------------------------------------------
template <class T>
CitizenCTS2000<T>::CitizenCTS2000()
{
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
	setConfigParameter(CHardware::Printer::FeedingAmount, 5);
	mDeviceName = "Citizen CT-S2000";
	setModelID('\x51');

	// модели
	mModelData.data().clear();
	mModelData.add(mModelID, true, mDeviceName);
}

//--------------------------------------------------------------------------------
template <class T>
void CitizenCTS2000<T>::setDeviceConfiguration(const QVariantMap & aConfiguration)
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
typedef SerialPOSPrinter<CitizenCTS2000<TSerialPrinterBase>> SerialCitizenCTS2000;
typedef                  CitizenCTS2000<TLibUSBPrinterBase>  LibUSBCitizenCTS2000;

//--------------------------------------------------------------------------------
