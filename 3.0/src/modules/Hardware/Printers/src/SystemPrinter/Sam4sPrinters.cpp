/* @file Принтеры Sam4s на системном драйвере. */

#pragma once

// windows
#include <windows.h>

// Project
#include "Sam4sPrinters.h"

//--------------------------------------------------------------------------------
Sam4s::Sam4s()
{
	mDeviceName = CSam4s::Models::Unknown;
	mIdName = "Sam4s";
	mSideMargin = 0.0;
	mAttributesDisabled.insert(PRINTER_STATUS_NOT_AVAILABLE);
	mAttributesDisabled.insert(PRINTER_STATUS_PAPER_PROBLEM);
}

//--------------------------------------------------------------------------------
QStringList Sam4s::getModelList()
{
	CSam4s::Models::CData modelData;
	QStringList result;

	foreach(auto data, modelData.data())
	{
		result << data.name;
	}

	return result;
}

//--------------------------------------------------------------------------------
bool Sam4s::isConnected()
{
	if (!SystemPrinter::isConnected())
	{
		return false;
	}

	QVariantMap deviceData = ISysUtils::getPrinterData(mLog, mPrinter.printerName());
	QString name = deviceData[CDeviceData::Name].toString();
	QStringList modelKeys = CSam4s::Models::Data.data().keys();

	auto it = std::find_if(modelKeys.begin(), modelKeys.end(), [&modelKeys, &name] (const QString & aKey) -> bool
		{ return name.contains(aKey, Qt::CaseInsensitive); });

	SBaseModelData modelData = (it != modelKeys.end()) ? CSam4s::Models::Data[*it] : SBaseModelData(CSam4s::Models::Unknown);
	mDeviceName = modelData.name;
	mVerified = modelData.verified;
	mModelCompatibility = it != modelKeys.end();

	return true;
}

//--------------------------------------------------------------------------------
