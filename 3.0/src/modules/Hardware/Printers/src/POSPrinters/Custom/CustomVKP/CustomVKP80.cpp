/* @file Принтер Custom VKP-80. */

#pragma once

#include "CustomVKP80.h"

//--------------------------------------------------------------------------------
template class CustomVKP80<TSerialPrinterBase>;

//--------------------------------------------------------------------------------
template<class T>
CustomVKP80<T>::CustomVKP80()
{
	mParameters = POSPrinters::CommonParameters;

	// статусы ошибок
	mParameters.errors.clear();

	mParameters.errors[20][3].insert('\x01', PrinterStatusCode::Error::PaperEnd);
	mParameters.errors[20][3].insert('\x04', PrinterStatusCode::Warning::PaperNearEnd);
	mParameters.errors[20][3].insert('\x20', PrinterStatusCode::OK::PaperInPresenter);
	//mParameters.errors[20][3].insert('\x40', PrinterStatusCode::Warning::PaperEndVirtual);

	mParameters.errors[20][4].insert('\x03', DeviceStatusCode::Error::CoverIsOpened);
	mParameters.errors[20][4].insert('\x08', PrinterStatusCode::OK::MotorMotion);

	mParameters.errors[20][5].insert('\x01', PrinterStatusCode::Error::Temperature);
	mParameters.errors[20][5].insert('\x02', PrinterStatusCode::Error::Port);
	mParameters.errors[20][5].insert('\x08', DeviceStatusCode::Error::PowerSupply);
	mParameters.errors[20][5].insert('\x40', PrinterStatusCode::Error::PaperJam);

	mParameters.errors[20][6].insert('\x01', PrinterStatusCode::Error::Cutter);
	mParameters.errors[20][6].insert('\x4C', DeviceStatusCode::Error::MemoryStorage);

	// теги
	mParameters.tagEngine.appendSingle(Tags::Type::Italic, "\x1B\x34", "\x01");
	mParameters.tagEngine.appendCommon(Tags::Type::DoubleWidth,  "\x1B\x21", "\x20");
	mParameters.tagEngine.appendCommon(Tags::Type::DoubleHeight, "\x1B\x21", "\x10");
	mParameters.tagEngine.appendSingle(Tags::Type::Amount, "", "\xFE");

	// параметры моделей
	mDeviceName = "Custom VKP-80";
	setModelID('\xB9');

	setConfigParameter(CHardware::Printer::FeedingAmount, 0);
	setConfigParameter(CHardwareSDK::Printer::LineSize, 42);

	// модели
	mModelData.data().clear();
	mModelData.add('\x5D', true, mDeviceName, "resolution 200 dpi");
	mModelData.add('\x5E', true, mDeviceName, "resolution 300 dpi");
	mModelData.add('\xB9', true, mDeviceName, "default");
	mModelData.add('\x95', true, mDeviceName, "modification VKP80II-EE");
}

//--------------------------------------------------------------------------------
template<class T>
void CustomVKP80<T>::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	EjectorPOS::setDeviceConfiguration(aConfiguration);

	if (aConfiguration.contains(CHardware::Codepage))
	{
		using namespace CHardware::Codepages;

		QString codepage = aConfiguration[CHardware::Codepage].toString();
		QString codecName = (codepage == CustomKZT) ? CustomKZT : CP866;

		mCodec = CodecByName[codecName];
	}
}

//--------------------------------------------------------------------------------
template<class T>
bool CustomVKP80<T>::updateParameters()
{
	if (!EjectorPOS<T>::updateParameters())
	{
		return false;
	}

	return mIOPort->write(CPOSPrinter::Command::SetNarrowFont);
}

//--------------------------------------------------------------------------------
template<class T>
bool CustomVKP80<T>::printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt)
{
	if (mOverflow)
	{
		PollingExpector expector;
		QByteArray data;

		if (!expector.wait([&]() -> bool { return getAnswer(data, 10) && data.contains(ASCII::XOn); }, CCustomVKP80::XOnWaiting))
		{
			return false;
		}

		mOverflow = false;
	}

	bool result = EjectorPOS::printReceipt(aLexemeReceipt);

	if (!getConfigParameter(CHardware::Printer::OutCall).toBool())
	{
		auto condition = [&]() -> bool {TStatusCodes statusCodes; return getStatus(statusCodes) && !statusCodes.contains(PrinterStatusCode::OK::MotorMotion); };
		PollingExpector().wait(condition, CCustomVKP80::PrintingWaiting);
	}

	return result;
}

//--------------------------------------------------------------------------------
template<class T>
bool CustomVKP80<T>::updateParametersOut()
{
	return updateParameters();
}

//--------------------------------------------------------------------------------
