/* @file Принтеры семейства Epson EU-T400. */

#pragma once

#include "EpsonEUT400.h"

//--------------------------------------------------------------------------------
EpsonEUT400::EpsonEUT400()
{
	using namespace SDK::Driver::IOPort::COM;

	mParameters = POSPrinters::CommonParameters;

	// параметры порта
	mPortParameters.insert(EParameters::BaudRate, POSPrinters::TSerialDevicePortParameter()
		<< EBaudRate::BR38400
		<< EBaudRate::BR19200
		<< EBaudRate::BR4800
		<< EBaudRate::BR9600);

	// параметры моделей
	mDeviceName = "Epson EU-T400";
	setModelID('\x27');
	mDeviceDataTimeout = CPOSPrinter::Timeouts::Info;
	setConfigParameter(CHardware::Printer::FeedingAmount, 0);
	setConfigParameter(CHardware::Printer::Commands::Cutting, CEpson::Command::CutBackFeed);

	// модели
	mModelData.data().clear();
	mModelData.add(mModelID, true, mDeviceName);
}

//--------------------------------------------------------------------------------
void EpsonEUT400::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	POSPrinter::setDeviceConfiguration(aConfiguration);

	if (getConfigParameter(CHardware::Printer::Settings::BackFeed).toString() != CHardwareSDK::Values::NotUse)
	{
		setConfigParameter(CHardware::Printer::FeedingAmount, 0);
		setConfigParameter(CHardware::Printer::Commands::Cutting, CEpson::Command::CutBackFeed);
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
		setConfigParameter(CHardware::Printer::Commands::Cutting, CEpson::Command::Cut);
	}
}

//--------------------------------------------------------------------------------
bool EpsonEUT400::updateParametersOut()
{
	if (!POSPrinter::updateParameters())
	{
		return false;
	}

	if (!mIOPort->write(CEpson::Command::ASBDisable))
	{
		return false;
	}

	QString loop = getConfigParameter(CHardware::Printer::Settings::Loop).toString();

	if (loop != CHardwareSDK::Values::Auto)
	{
		QByteArray loopCommand = (loop == CHardwareSDK::Values::Use) ? CEpson::Command::LoopEnable : CEpson::Command::LoopDisable;

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

	if (!getMemorySwitch(CEpson::MemorySwitch::ReceiptProcessing2, receiptProcessing2))
	{
		return false;
	}

	QString configBackFeed = getConfigParameter(CHardware::Printer::Settings::BackFeed).toString();
	bool mswBackFeed = receiptProcessing2 == ProtocolUtils::mask(receiptProcessing2, CEpson::MemorySwitch::BackFeedMask);

	if (((configBackFeed == CHardwareSDK::Values::NotUse) && mswBackFeed) || (configBackFeed == CHardwareSDK::Values::Use) || mswBackFeed)
	{
		QString backFeedMask = (configBackFeed == CHardwareSDK::Values::NotUse) ? CEpson::MemorySwitch::NoBackFeedMask : CEpson::MemorySwitch::ReceiptProcessing2Mask;
		char newReceiptProcessing2 = ProtocolUtils::mask(receiptProcessing2, backFeedMask);

		if ((receiptProcessing2 != newReceiptProcessing2) && !setMemorySwitch(CEpson::MemorySwitch::ReceiptProcessing2, newReceiptProcessing2))
		{
			return false;
		}
	}

	return updateParametersOut();
}

//--------------------------------------------------------------------------------
