/* @file Базовый ФР Pay на протоколе Штрих. */

#pragma once

#include "PayFRBase.h"

//--------------------------------------------------------------------------------
template class PayFRBase<ShtrihOnlineFRBase<ShtrihTCPFRBase>>;
template class PayFRBase<ShtrihOnlineFRBase<ShtrihSerialFRBase>>;

//--------------------------------------------------------------------------------
template<class T>
PayFRBase<T>::PayFRBase(): mPrinterModelId(0)
{
	mOFDFiscalFields << CFR::FiscalFields::Cashier;
}

//--------------------------------------------------------------------------------
template<class T>
void PayFRBase<T>::processDeviceData()
{
	ShtrihRetractorFRLite<T>::processDeviceData();

	QString SDCardData = getDeviceParameter(CDeviceData::SDCard).toString();
	bool SDCardError = SDCardData.isEmpty() || SDCardData.startsWith(CDeviceData::Error) || SDCardData.startsWith(CDeviceData::NotConnected);
	mCanProcessZBuffer = mCanProcessZBuffer && !SDCardError && (mModelData.date >= CShtrihOnlineFR::MinFWDate::ZBuffer);
}

//--------------------------------------------------------------------------------
template<class T>
void PayFRBase<T>::appendStatusCodes(ushort aFlags, TStatusCodes & aStatusCodes)
{
	ShtrihRetractorFRLite<T>::appendStatusCodes(aFlags, aStatusCodes);

	bool paperWeightSensor = (~aFlags & CShtrihFR::Statuses::WeightSensor::NoChequePaper);
	bool useRemotePaperSensor = getConfigParameter(CHardware::Printer::Settings::RemotePaperSensor).toString() == CHardwareSDK::Values::Use;
	bool hasPNESensor = CPayPrinters::Models[mPrinterModelId].hasPNESensor;

	if (paperWeightSensor && useRemotePaperSensor && hasPNESensor)
	{
		aStatusCodes.insert(PrinterStatusCode::Warning::PaperNearEnd);
		toLog(LogLevel::Warning, "ShtrihFR: Paper near end, report weight sensor");
	}
}

//--------------------------------------------------------------------------------
template<class T>
bool PayFRBase<T>::execZReport(bool aAuto)
{
	if (mCanProcessZBuffer && ShtrihFRBase::execZReport(aAuto))
	{
		return true;
	}

	return ShtrihOnlineFRBase::execZReport(aAuto);
}

//--------------------------------------------------------------------------------
template<class T>
bool PayFRBase<T>::retract()
{
	return processCommand(CShtrihFR::Commands::Cut, QByteArray(1, CShtrihFR::FullCutting));
}

//--------------------------------------------------------------------------------
