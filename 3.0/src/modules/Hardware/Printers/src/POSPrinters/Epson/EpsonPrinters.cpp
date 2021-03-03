/* @file Принтеры семейства Epson. */

#pragma once

#include "EpsonPrinters.h"
#include "Hardware/Printers/Sam4sModels.h"

//--------------------------------------------------------------------------------
template class EpsonPrinter<TSerialPOSPrinter>;
template class EpsonPrinter<TTCPPOSPrinter>;

//--------------------------------------------------------------------------------
template <class T>
EpsonPrinter<T>::EpsonPrinter(): mDeviceDataTimeout(CEpson::Timeouts::DeviceData)
{
	mParameters = POSPrinters::CommonParameters;

	// статусы ошибок
	mParameters.errors.clear();

	mParameters.errors[1][1].insert('\x08', DeviceStatusCode::Error::Unknown);

	mParameters.errors[2][1].insert('\x04', DeviceStatusCode::Error::CoverIsOpened);
	mParameters.errors[2][1].insert('\x20', PrinterStatusCode::Error::PaperEnd);
	mParameters.errors[2][1].insert('\x40', DeviceStatusCode::Error::Unknown);

	mParameters.errors[3][1].insert('\x04', DeviceStatusCode::Error::MechanismPosition);
	mParameters.errors[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
	mParameters.errors[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

	mParameters.errors[4][1].insert('\x08', PrinterStatusCode::Warning::PaperNearEnd);
	mParameters.errors[4][1].insert('\x40', PrinterStatusCode::Error::PaperEnd);

	// теги
	mParameters.tagEngine.appendCommon(Tags::Type::DoubleWidth,  "\x1B\x21", "\x20");
	mParameters.tagEngine.appendCommon(Tags::Type::DoubleHeight, "\x1B\x21", "\x10");

	// параметры моделей
	mIOMessageLogging = ELoggingType::Write;
}

//--------------------------------------------------------------------------------
template <class T>
bool EpsonPrinter<T>::reset()
{
	if (!mIOPort->write(CPOSPrinter::Command::Initialize) || !mIOPort->write(CPOSPrinter::Command::SetEnabled))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to reset printer");
		return false;
	}

	SleepHelper::msleep(CEpson::MemorySwitch::Pause);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool EpsonPrinter<T>::getMemorySwitch(char aNumber, char & aValue)
{
	if (!reset() || !mIOPort->write(CEpson::Command::GetMemorySwitch + aNumber))
	{
		return false;
	}

	QByteArray answer;

	QTime clockTimer;
	clockTimer.restart();

	do
	{
		QByteArray data;

		if (!mIOPort->read(data, 20))
		{
			return false;
		}

		answer.append(data);
	}
	while ((clockTimer.elapsed() < CEpson::Timeouts::MemorySwitch) && !answer.endsWith(ASCII::NUL));

	toLog(LogLevel::Normal, QString("%1: << {%2}").arg(mDeviceName).arg(answer.toHex().data()));

	if ((answer.size() != CEpson::MemorySwitch::AnswerSize) || !answer.startsWith(CEpson::MemorySwitch::Prefix) || !answer.endsWith(CEpson::MemorySwitch::Postfix))
	{
		toLog(LogLevel::Error, QString("%1: Failed to get memory switch %2 due to wrong answer").arg(mDeviceName).arg(int(aNumber)));
		return false;
	}

	bool OK;
	aValue = char(answer.mid(2, 8).toInt(&OK, 2));

	if (!OK)
	{
		toLog(LogLevel::Error, QString("%1: Failed to parse answer for memory switch %2").arg(mDeviceName).arg(int(aNumber)));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool EpsonPrinter<T>::setMemorySwitch(char aNumber, char aValue)
{
	if (!reset())
	{
		return false;
	}

	QByteArray answer;
	bool result = mIOPort->write(CEpson::Command::EnterUserMode) && getAnswer(answer, 500) && (answer == CEpson::MemorySwitch::AnswerForEnter);
	SleepHelper::msleep(CEpson::MemorySwitch::Pause);

	if (!result)
	{
		mIOPort->write(CEpson::Command::ExitUserMode);
		SleepHelper::msleep(CEpson::MemorySwitch::Pause);
		toLog(LogLevel::Error, mDeviceName + ": Failed to enter user mode");

		return false;
	}

	QByteArray command = CEpson::Command::SetMemorySwitch + aNumber + QByteArray::number(aValue, 2).right(8);

	if (!mIOPort->write(command))
	{
		toLog(LogLevel::Error, QString("%1: Failed to set memory switch %2").arg(mDeviceName).arg(int(aNumber)));
		return false;
	}

	SleepHelper::msleep(CEpson::MemorySwitch::Pause);

	if (!mIOPort->write(CEpson::Command::ExitUserMode))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to exit user mode");
		return false;
	}

	SleepHelper::msleep(CEpson::MemorySwitch::ExitPause);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool EpsonPrinter<T>::getDeviceData(const QByteArray & aCommand, QByteArray & aAnswer)
{
	aAnswer.clear();
	setPortLoggingType(ELoggingType::Write);

	QTime timer;
	timer.start();

	if (!mIOPort->write(aCommand))
	{
		return false;
	}

	while (timer.elapsed() < mDeviceDataTimeout)
	{
		QByteArray data;

		if (!getAnswer(data, 10))
		{
			return false;
		}

		aAnswer += data;
	}

	aAnswer.replace('\x00', "").replace('\x5f', "");

	toLog(LogLevel::Normal, QString("%1: << {%2}").arg(mDeviceName).arg(aAnswer.toHex().data()));
	setPortLoggingType(ELoggingType::ReadWrite);

	return !aAnswer.isEmpty();
}

//--------------------------------------------------------------------------------
template <class T>
void EpsonPrinter<T>::processDeviceData()
{
	QByteArray answer;

	if (mIOPort->write(CPOSPrinter::Command::GetTypeId) && getAnswer(answer, CPOSPrinter::Timeouts::Info) && (answer.size() == 1))
	{
		setDeviceParameter(CDeviceData::Printers::Unicode,  ProtocolUtils::getBit(answer, 0));
		setDeviceParameter(CDeviceData::Printers::Cutter,   ProtocolUtils::getBit(answer, 1));
		setDeviceParameter(CDeviceData::Printers::BMSensor, ProtocolUtils::getBit(answer, 2));
	}

	if (getDeviceData(CEpson::Command::GetVersion, answer))
	{
		setDeviceParameter(CDeviceData::Firmware, answer);
	}

	CEpson::CFontType fontType;

	if (getDeviceData(CEpson::Command::GetFont, answer) && fontType.data().contains(answer))
	{
		setDeviceParameter(CDeviceData::Printers::Font, fontType[answer]);
	}

	CEpson::CMemorySize memorySize;

	if (getDeviceData(CEpson::Command::GetMemorySize, answer) && memorySize.data().contains(answer[0]))
	{
		setDeviceParameter(CDeviceData::Memory, QString("%1 Mbits").arg(memorySize[answer[0]]));
	}

	if (getDeviceData(CEpson::Command::GetOptions, answer) && (answer.size() == 1))
	{
		setDeviceParameter(CDeviceData::Printers::Presenter,   ProtocolUtils::getBit(answer, 0));
		setDeviceParameter(CDeviceData::Printers::PaperSupply, ProtocolUtils::getBit(answer, 4));
	}
}

//--------------------------------------------------------------------------------
