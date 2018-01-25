/* @file Принтер Custom VKP-80. */

#pragma once

#include "CustomVKP80.h"

//--------------------------------------------------------------------------------
CustomVKP80::CustomVKP80()
{
	POSPrinters::SParameters parameters(mModelData.getDefault().parameters);

	// статусы ошибок
	parameters.errors->data().clear();

	parameters.errors->data()[20][3].insert('\x01', PrinterStatusCode::Error::PaperEnd);
	parameters.errors->data()[20][3].insert('\x04', PrinterStatusCode::Warning::PaperNearEnd);
	parameters.errors->data()[20][3].insert('\x20', PrinterStatusCode::OK::PaperInPresenter);
	//parameters.errors->data()[20][3].insert('\x40', PrinterStatusCode::Warning::PaperEndVirtual);

	parameters.errors->data()[20][4].insert('\x03', DeviceStatusCode::Error::CoverIsOpened);
	parameters.errors->data()[20][4].insert('\x08', PrinterStatusCode::OK::MotorMotion);

	parameters.errors->data()[20][5].insert('\x01', PrinterStatusCode::Error::Temperature);
	parameters.errors->data()[20][5].insert('\x02', PrinterStatusCode::Error::Port);
	parameters.errors->data()[20][5].insert('\x08', DeviceStatusCode::Error::PowerSupply);
	parameters.errors->data()[20][5].insert('\x40', PrinterStatusCode::Error::PaperJam);

	parameters.errors->data()[20][6].insert('\x01', PrinterStatusCode::Error::Cutter);
	parameters.errors->data()[20][6].insert('\x4C', DeviceStatusCode::Error::MemoryStorage);

	// теги
	parameters.tagEngine->appendSingle(Tags::Type::Italic, "\x1B\x34", "\x01");
	parameters.tagEngine->appendCommon(Tags::Type::DoubleWidth,  "\x1B\x21", "\x20");
	parameters.tagEngine->appendCommon(Tags::Type::DoubleHeight, "\x1B\x21", "\x10");

	// параметры моделей
	mDeviceName = "Custom VKP-80";
	mModelID = '\xB9';

	setConfigParameter(CHardware::Printer::FeedingAmount, 0);

	// модели
	mModelData.data().clear();
	mModelData.add('\x5D', true, mDeviceName, parameters, "resolution 200 dpi");
	mModelData.add('\x5E', true, mDeviceName, parameters, "resolution 300 dpi");
	mModelData.add('\xB9', true, mDeviceName, parameters, "default");
	mModelData.add('\x95', true, mDeviceName, parameters, "modification VKP80II-EE");
}

//--------------------------------------------------------------------------------
bool CustomVKP80::printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt)
{
	if (mOverflow)
	{
		PollingExpector expector;
		QByteArray data;

		if (!expector.wait([&]() -> bool { return getAnswer(data, 10) && data.contains(ASCII::XOn); }, CCustomVKP80::PollingInterval, CCustomVKP80::XOnXOffTimeout))
		{
			return false;
		}

		mOverflow = false;
	}

	bool result = EjectorPOS::printReceipt(aLexemeReceipt);

	if (!getConfigParameter(CHardware::Printer::OutCall).toBool())
	{
		auto condition = [&]() -> bool {TStatusCodes statusCodes; return getStatus(statusCodes) && !statusCodes.contains(PrinterStatusCode::OK::MotorMotion); };
		PollingExpector().wait(condition, CCustomVKP80::PollingInterval, CCustomVKP80::PrintingEndTimeout);
	}

	return result;
}

//--------------------------------------------------------------------------------
bool CustomVKP80::updateParametersOut()
{
	return updateParameters();
}

//--------------------------------------------------------------------------------
