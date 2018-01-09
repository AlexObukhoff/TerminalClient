/* @file ФР Пэй на протоколе Штрих. */

#pragma once

#include "PayOnlineFR.h"

//--------------------------------------------------------------------------------
template class PayOnlineFR<ShtrihOnlineFRBase<ShtrihTCPFRBase>>;
template class PayOnlineFR<ShtrihOnlineFRBase<ShtrihSerialFRBase>>;

//--------------------------------------------------------------------------------
template<class T>
PayOnlineFR<T>::PayOnlineFR()
{
	mDeviceName = CShtrihFR::Models::OnlineRetractorDefault;
	mOFDFiscalParameters << SDK::Driver::FiscalFields::Cashier;

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

	mCanProcessZBuffer = mModelData.date >= CShtrihOnlineFR::MinZBufferFirmwareDate;
	QByteArray data;

	if (getFRParameter(CShtrihOnlineFR::FRParameters::PrinterModel, data) && CPayOnlineFR::PrinterModels.data().contains(data[0]))
	{
		setDeviceParameter(CDeviceData::FR::Printer, CPayOnlineFR::PrinterModels[data[0]]);
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
