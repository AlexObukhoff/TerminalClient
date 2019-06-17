/* @file Принтер Citizen PPU-700. */

#pragma once

#include "CitizenPPU700.h"

//--------------------------------------------------------------------------------
CitizenPPU700::CitizenPPU700()
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

	parameters.errors->data()[3][1].insert('\x04', PrinterStatusCode::Error::Presenter);
	parameters.errors->data()[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
	parameters.errors->data()[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

	parameters.errors->data()[4][1].insert('\x0C', PrinterStatusCode::Warning::PaperNearEnd);
	parameters.errors->data()[4][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
	parameters.errors->data()[4][1].insert('\x40', PrinterStatusCode::OK::PaperInPresenter);

	parameters.errors->data()[5][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
	parameters.errors->data()[5][1].insert('\x08', PrinterStatusCode::Error::Temperature);
	parameters.errors->data()[5][1].insert('\x60', DeviceStatusCode::Error::PowerSupply);

	parameters.errors->data()[6][1].insert('\x0C', DeviceStatusCode::Error::MemoryStorage);
	parameters.errors->data()[6][1].insert('\x20', PrinterStatusCode::Error::Presenter);
	parameters.errors->data()[6][1].insert('\x40', DeviceStatusCode::Error::Electronic);

	// параметры моделей
	mDeviceName = "Citizen PPU-700";
	mModelID = '\x75';
	setConfigParameter(CHardware::Printer::FeedingAmount, 4);

	// модели
	mModelData.data().clear();
	mModelData.add(mModelID, true, mDeviceName, parameters);
	mPortParameters = parameters.portSettings->data();
	mMaxBadAnswers = 4;
	mOptionMSW = false;
}

//--------------------------------------------------------------------------------
bool CitizenPPU700::isConnected()
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
void CitizenPPU700::processDeviceData()
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
bool CitizenPPU700::getNULStoppedAnswer(QByteArray & aAnswer, int aTimeout) const
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
void CitizenPPU700::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	EjectorPOS::setDeviceConfiguration(aConfiguration);

	int lineSpacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing).toInt();

	int feeding = 4;
	     if (lineSpacing >= 50) feeding = 1;
	else if (lineSpacing >= 24) feeding = 2;
	else if (lineSpacing >=  8) feeding = 3;

	setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
}

//--------------------------------------------------------------------------------
