/* @file Принтер Citizen PPU-700. */

#pragma once

#include "CitizenPPU700.h"

//--------------------------------------------------------------------------------
template class SerialPOSPrinter<CitizenPPU700<TSerialPrinterBase>>;
template class SerialPOSPrinter<CitizenPPU700II<TSerialPrinterBase>>;

//--------------------------------------------------------------------------------
template<class T>
CitizenPPU700<T>::CitizenPPU700()
{
	// статусы ошибок
	mParameters.errors.clear();

	mParameters.errors[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);

	mParameters.errors[2][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
	mParameters.errors[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
	mParameters.errors[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);

	mParameters.errors[3][1].insert('\x04', PrinterStatusCode::Error::Presenter);
	mParameters.errors[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
	mParameters.errors[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

	mParameters.errors[4][1].insert('\x0C', PrinterStatusCode::Warning::PaperNearEnd);
	mParameters.errors[4][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
	mParameters.errors[4][1].insert('\x40', PrinterStatusCode::OK::PaperInPresenter);

	mParameters.errors[5][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
	mParameters.errors[5][1].insert('\x08', PrinterStatusCode::Error::Temperature);
	mParameters.errors[5][1].insert('\x60', DeviceStatusCode::Error::PowerSupply);

	mParameters.errors[6][1].insert('\x0C', DeviceStatusCode::Error::MemoryStorage);
	mParameters.errors[6][1].insert('\x20', PrinterStatusCode::Error::Presenter);
	mParameters.errors[6][1].insert('\x40', DeviceStatusCode::Error::Electronic);

	// параметры моделей
	mDeviceName = "Citizen PPU-700";
	mModelID = '\x75';
	setConfigParameter(CHardware::Printer::FeedingAmount, 4);

	// модели
	mModelData.data().clear();
	mModelData.add(mModelID, true, mDeviceName);
	mMaxBadAnswers = 4;
	mOptionMSW = false;
}

//--------------------------------------------------------------------------------
template<class T>
bool CitizenPPU700<T>::isConnected()
{
	if (!CitizenBase<EjectorPOS>::isConnected())
	{
		return false;
	}

	if (mModelCompatibility)
	{
		QByteArray answer;

		if (!mIOPort->write(CCitizenPPU700::Command::GetMemorySwitch5) || !mIOPort->read(answer, CCitizenPPU700::MemorySwitches::ReadingTimeout, CCitizenPPU700::MemorySwitches::AnswerSize))
		{
			return false;
		}

		toLog(LogLevel::Normal, QString("%1: << {%2}").arg(mDeviceName).arg(answer.toHex().data()));

		mModelCompatibility = mOptionMSW == !answer.isEmpty();
		mDeviceName = answer.isEmpty() ? "Citizen PPU-700" : "Citizen PPU-700II";
	}

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
void CitizenPPU700<T>::processDeviceData()
{
	EjectorPOS::processDeviceData();
	QByteArray answer;

	if (mIOPort->write(CCitizenPPU700::Command::GetFirmware) && getNULStoppedAnswer(answer, CPOSPrinter::Timeouts::Info))
	{
		setDeviceParameter(CDeviceData::Firmware, answer);
	}

	if (mIOPort->write(CCitizenPPU700::Command::GetSerialNumber) && getNULStoppedAnswer(answer, CPOSPrinter::Timeouts::Info))
	{
		setDeviceParameter(CDeviceData::SerialNumber, answer);
	}
}

//--------------------------------------------------------------------------------
template<class T>
bool CitizenPPU700<T>::getNULStoppedAnswer(QByteArray & aAnswer, int aTimeout) const
{
	QVariantMap configuration;
	configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(ELoggingType::Write));
	mIOPort->setDeviceConfiguration(configuration);

	aAnswer.clear();

	QTime timer;
	timer.start();

	do
	{
		QByteArray data;

		if (!mIOPort->read(data, 10))
		{
			return false;
		}

		aAnswer.append(data);
	}
	while (!aAnswer.endsWith(ASCII::NUL) && (timer.elapsed() < aTimeout));

	toLog(aAnswer.isEmpty() ? LogLevel::Warning : LogLevel::Normal, QString("%1: << {%2}").arg(mDeviceName).arg(aAnswer.toHex().data()));

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
void CitizenPPU700<T>::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	EjectorPOS::setDeviceConfiguration(aConfiguration);

	int lineSpacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();

	int feeding = 4;
	     if (lineSpacing >= 202) feeding = 0;
	else if (lineSpacing >= 102) feeding = 1;
	else if (lineSpacing >=  72) feeding = 2;
	else if (lineSpacing >=  52) feeding = 3;

	setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
}

//--------------------------------------------------------------------------------
