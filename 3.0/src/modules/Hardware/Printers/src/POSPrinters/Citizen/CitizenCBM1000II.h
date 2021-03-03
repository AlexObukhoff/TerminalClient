/* @file Принтер Citizen CBM-1000II. */

#pragma once

#include "CitizenBase.h"

//--------------------------------------------------------------------------------
template <class T>
class CitizenCBM1000II : public CitizenBase<POSPrinter<T>>
{
	SET_SUBSERIES("CitizenCBM1000II")

public:
	CitizenCBM1000II();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);
};

//--------------------------------------------------------------------------------
template <class T>
CitizenCBM1000II<T>::CitizenCBM1000II()
{
	using namespace SDK::Driver::IOPort::COM;

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
	mDeviceName = "Citizen CBM-1000II";
	setModelID('\x30');
	setConfigParameter(CHardware::Printer::FeedingAmount, 3);

	// модели
	mModelData.data().clear();
	mModelData.add(mModelID, true, mDeviceName);

	setConfigParameter(CHardware::Printer::FeedingAmount, 5);
}

//--------------------------------------------------------------------------------
template <class T>
void CitizenCBM1000II<T>::setDeviceConfiguration(const QVariantMap & aConfiguration)
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
class SerialCitizenCBM1000II : public SerialPOSPrinter<CitizenCBM1000II<TSerialPrinterBase>>
{
public:
	SerialCitizenCBM1000II()
	{
		using namespace SDK::Driver::IOPort::COM;

		// параметры порта
		mPortParameters.insert(EParameters::BaudRate, POSPrinters::TSerialDevicePortParameter()
			<< EBaudRate::BR38400
			<< EBaudRate::BR19200
			<< EBaudRate::BR4800
			<< EBaudRate::BR9600);
	}
};

//--------------------------------------------------------------------------------
typedef CitizenCBM1000II<TLibUSBPrinterBase> LibUSBCitizenCBM1000II;

//--------------------------------------------------------------------------------
