/* @file ФР Пэй на протоколе Штрих. */

#pragma once

#include "PayOnlineFR.h"

//--------------------------------------------------------------------------------
template class PayOnlineFR<ShtrihOnlineFRBase<ShtrihTCPFRBase>>;
template class PayOnlineFR<ShtrihOnlineFRBase<ShtrihSerialFRBase>>;

//--------------------------------------------------------------------------------
template<class T>
PayOnlineFR<T>::PayOnlineFR(): mPrinterModelId(0)
{
	mDeviceName = CShtrihFR::Models::OnlineRetractorDefault;
	mOFDFiscalParameters << CFR::FiscalFields::Cashier;

	mSupportedModels = getModelList();
}

//--------------------------------------------------------------------------------
template<class T>
QStringList PayOnlineFR<T>::getModelList()
{
	using namespace CShtrihFR::Models;

	return CData().getModelList(CShtrihFR::TIds() << ID::PayOnline01FA << ID::PayVKP80KFA);
}

//--------------------------------------------------------------------------------
template<class T>
bool PayOnlineFR<T>::updateParameters()
{
	if (!ShtrihRetractorFRLite<T>::updateParameters())
	{
		return false;
	}

	QByteArray data;

	if (!getFRParameter(CShtrihOnlineFR::FRParameters::AutomaticNumber, data))
	{
		return false;
	}

	setDeviceParameter(CDeviceData::FR::AutomaticNumber, ProtocolUtils::clean(data));

	return true;
}

//--------------------------------------------------------------------------------
template<class T>
void PayOnlineFR<T>::processDeviceData()
{
	ShtrihRetractorFRLite<T>::processDeviceData();

	mCanProcessZBuffer = mModelData.date >= CShtrihOnlineFR::MinFWDate::ZBuffer;
	QByteArray data;

	if (getFRParameter(CShtrihOnlineFR::FRParameters::PrinterModel, data) && CPayOnlineFR::PrinterModels.data().contains(data[0]))
	{
		mPrinterModelId = uchar(data[0]);
		setDeviceParameter(CDeviceData::FR::Printer, CPayOnlineFR::PrinterModels[mPrinterModelId].name);
	}
}

//--------------------------------------------------------------------------------
template<class T>
void PayOnlineFR<T>::appendStatusCodes(ushort aFlags, TStatusCodes & aStatusCodes)
{
	ShtrihRetractorFRLite<T>::appendStatusCodes(aFlags, aStatusCodes);

	bool paperWeightSensor = (~aFlags & CShtrihFR::Statuses::WeightSensor::NoChequePaper);
	bool useRemotePaperSensor = getConfigParameter(CHardware::Printer::Settings::RemotePaperSensor).toString() == CHardware::Values::Use;
	bool hasPNESensor = CPayOnlineFR::PrinterModels[mPrinterModelId].hasPNESensor;

	if (paperWeightSensor && useRemotePaperSensor && hasPNESensor)
	{
		aStatusCodes.insert(PrinterStatusCode::Warning::PaperNearEnd);
		toLog(LogLevel::Warning, "ShtrihFR: Paper near end, report weight sensor");
	}
}

//--------------------------------------------------------------------------------
template<class T>
bool PayOnlineFR<T>::execZReport(bool aAuto)
{
	if (mCanProcessZBuffer && ShtrihFRBase::execZReport(aAuto))
	{
		return true;
	}

	return ShtrihOnlineFRBase::execZReport(aAuto);
}

//--------------------------------------------------------------------------------
