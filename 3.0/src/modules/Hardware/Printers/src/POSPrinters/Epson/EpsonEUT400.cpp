/* @file Принтеры семейства Epson EU-T400. */

#pragma once

#include "EpsonEUT400.h"
#include "EpsonConstants.h"

//--------------------------------------------------------------------------------
EpsonEUT400::EpsonEUT400()
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

	parameters.errors->data()[3][1].insert('\x04', DeviceStatusCode::Error::MechanismPosition);
	parameters.errors->data()[3][1].insert('\x08', PrinterStatusCode::Error::Cutter);
	parameters.errors->data()[3][1].insert('\x60', DeviceStatusCode::Error::Unknown);

	parameters.errors->data()[4][1].insert('\x08', PrinterStatusCode::Warning::PaperNearEnd);
	parameters.errors->data()[4][1].insert('\x40', PrinterStatusCode::Error::PaperEnd);

	// теги
	parameters.tagEngine->appendCommon(Tags::Type::DoubleWidth,  "\x1B\x21", "\x20");
	parameters.tagEngine->appendCommon(Tags::Type::DoubleHeight, "\x1B\x21", "\x10");

	// параметры моделей
	mDeviceName = "Epson EU-T400";
	mModelID = '\x27';
	mIOMessageLogging = ELoggingType::Write;
	setConfigParameter(CHardware::Printer::FeedingAmount, 0);
	setConfigParameter(CHardware::Printer::Commands::Cutting, CEpsonEUT400::Command::CutBackFeed);

	// модели
	mModelData.data().clear();
	mModelData.add(mModelID, true, mDeviceName, parameters);
	mPortParameters = parameters.portSettings->data();
}

//--------------------------------------------------------------------------------
void EpsonEUT400::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	POSPrinter::setDeviceConfiguration(aConfiguration);

	if (getConfigParameter(CHardware::Printer::Settings::BackFeed).toString() != CHardwareSDK::Values::NotUse)
	{
		setConfigParameter(CHardware::Printer::FeedingAmount, 0);
		setConfigParameter(CHardware::Printer::Commands::Cutting, CEpsonEUT400::Command::CutBackFeed);
	}
	else
	{
		int lineSpacing = getConfigParameter(CHardware::Printer::Settings::LineSpacing, 0).toInt();

		int feeding = 6;
			 if (lineSpacing >= 50) feeding = 2;
		else if (lineSpacing >= 38) feeding = 3;
		else if (lineSpacing >= 30) feeding = 4;
		else if (lineSpacing >= 25) feeding = 5;

		setConfigParameter(CHardware::Printer::FeedingAmount, feeding);
		setConfigParameter(CHardware::Printer::Commands::Cutting, CEpsonEUT400::Command::Cut);
	}
}

//--------------------------------------------------------------------------------
bool EpsonEUT400::updateParametersOut()
{
	if (!POSPrinter::updateParameters())
	{
		return false;
	}

	if (!mIOPort->write(CEpsonEUT400::Command::ASBDisable))
	{
		return false;
	}

	QString loop = getConfigParameter(CHardware::Printer::Settings::Loop).toString();

	if (loop != CHardwareSDK::Values::Auto)
	{
		QByteArray loopCommand = (loop == CHardwareSDK::Values::Use) ? CEpsonEUT400::Command::LoopEnable : CEpsonEUT400::Command::LoopDisable;

		if (!mIOPort->write(loopCommand))
		{
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool EpsonEUT400::updateParameters()
{
	char receiptProcessing2;

	if (!getMemorySwitch(CEpsonEUT400::MemorySwitch::ReceiptProcessing2, receiptProcessing2))
	{
		return false;
	}

	QString configBackFeed = getConfigParameter(CHardware::Printer::Settings::BackFeed).toString();
	bool mswBackFeed = receiptProcessing2 == ProtocolUtils::mask(receiptProcessing2, CEpsonEUT400::MemorySwitch::BackFeedMask);

	if (((configBackFeed == CHardwareSDK::Values::NotUse) && mswBackFeed) || (configBackFeed == CHardwareSDK::Values::Use) || mswBackFeed)
	{
		QString backFeedMask = (configBackFeed == CHardwareSDK::Values::NotUse) ? CEpsonEUT400::MemorySwitch::NoBackFeedMask : CEpsonEUT400::MemorySwitch::ReceiptProcessing2Mask;
		char newReceiptProcessing2 = ProtocolUtils::mask(receiptProcessing2, backFeedMask);

		if ((receiptProcessing2 != newReceiptProcessing2) && !setMemorySwitch(CEpsonEUT400::MemorySwitch::ReceiptProcessing2, newReceiptProcessing2))
		{
			return false;
		}
	}

	return updateParametersOut();
}

//--------------------------------------------------------------------------------
bool EpsonEUT400::reset()
{
	if (!mIOPort->write(CPOSPrinter::Command::Initialize) || !mIOPort->write(CPOSPrinter::Command::SetEnabled))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to reset printer");
		return false;
	}

	SleepHelper::msleep(CEpsonEUT400::MemorySwitch::Pause);

	return true;
}

//--------------------------------------------------------------------------------
bool EpsonEUT400::getMemorySwitch(char aNumber, char & aValue)
{
	if (!reset() || !mIOPort->write(CEpsonEUT400::Command::GetMemorySwitch + aNumber))
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
	while ((clockTimer.elapsed() < CEpsonEUT400::MemorySwitch::TimeoutReading) && !answer.endsWith(ASCII::NUL));

	toLog(LogLevel::Normal, QString("%1: << {%2}").arg(mDeviceName).arg(answer.toHex().data()));

	if ((answer.size() != CEpsonEUT400::MemorySwitch::AnswerSize) || !answer.startsWith(CEpsonEUT400::MemorySwitch::Prefix) || !answer.endsWith(CEpsonEUT400::MemorySwitch::Postfix))
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
bool EpsonEUT400::setMemorySwitch(char aNumber, char aValue)
{
	if (!reset())
	{
		return false;
	}

	QByteArray answer;
	bool result = mIOPort->write(CEpsonEUT400::Command::EnterUserMode) && getAnswer(answer, 500) && (answer == CEpsonEUT400::MemorySwitch::AnswerForEnter);
	SleepHelper::msleep(CEpsonEUT400::MemorySwitch::Pause);

	if (!result)
	{
		mIOPort->write(CEpsonEUT400::Command::ExitUserMode);
		SleepHelper::msleep(CEpsonEUT400::MemorySwitch::Pause);
		toLog(LogLevel::Error, mDeviceName + ": Failed to enter user mode");

		return false;
	}

	QByteArray command = CEpsonEUT400::Command::SetMemorySwitch + aNumber + QByteArray::number(aValue, 2).right(8);

	if (!mIOPort->write(command))
	{
		toLog(LogLevel::Error, QString("%1: Failed to set memory switch %2").arg(mDeviceName).arg(int(aNumber)));
		return false;
	}

	SleepHelper::msleep(CEpsonEUT400::MemorySwitch::Pause);

	if (!mIOPort->write(CEpsonEUT400::Command::ExitUserMode))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to exit user mode");
		return false;
	}

	SleepHelper::msleep(CEpsonEUT400::MemorySwitch::ExitPause);

	return true;
}

//--------------------------------------------------------------------------------
void EpsonEUT400::processDeviceData()
{
	QByteArray answer;

	if (mIOPort->write(CPOSPrinter::Command::GetTypeId) && getAnswer(answer, CPOSPrinter::Timeouts::Info) && (answer.size() == 1))
	{
		setDeviceParameter(CDeviceData::Printers::Unicode,  ProtocolUtils::getBit(answer, 0));
		setDeviceParameter(CDeviceData::Printers::Cutter,   ProtocolUtils::getBit(answer, 1));
		setDeviceParameter(CDeviceData::Printers::BMSensor, ProtocolUtils::getBit(answer, 2));
	}

	auto processCommand = [&] (const QByteArray & aCommand) ->bool { if (!mIOPort->write(aCommand) || !getAnswer(answer, CPOSPrinter::Timeouts::Info)) return false;
		answer.replace('\x00', "").replace('\x5f', ""); return !answer.isEmpty(); };

	if (!processCommand(CEpsonEUT400::Command::GetVersion))
	{
		setDeviceParameter(CDeviceData::Firmware, answer);
	}

	CEpsonEUT400::CFontType fontType;

	if (processCommand(CEpsonEUT400::Command::GetFont) && fontType.data().contains(answer))
	{
		setDeviceParameter(CDeviceData::Printers::Font, fontType[answer]);
	}

	CEpsonEUT400::CMemorySize memorySize;

	if (processCommand(CEpsonEUT400::Command::GetMemorySize) && memorySize.data().contains(answer[0]))
	{
		setDeviceParameter(CDeviceData::Memory, QString("%1 Mbits").arg(memorySize[answer[0]]));
	}

	if (processCommand(CEpsonEUT400::Command::GetOptions) && (answer.size() == 1))
	{
		setDeviceParameter(CDeviceData::Printers::Presenter,   ProtocolUtils::getBit(answer, 0));
		setDeviceParameter(CDeviceData::Printers::PaperSupply, ProtocolUtils::getBit(answer, 4));
	}
}

//--------------------------------------------------------------------------------
