/* @file ФР PayVKP-80K-FA на протоколе Штрих. */

#pragma once

#include "PayFRBase.h"

//--------------------------------------------------------------------------------
template<class T>
class PayVKP80FA : public PayFRBase<T>
{
	SET_SUBSERIES("PayVKP80FA")
	SET_VCOM_DATA(None, ConnectionTypes::COMOnly, None)

public:
	PayVKP80FA()
	{
		mDeviceName = CShtrihFR::Models::CData()[CShtrihFR::Models::ID::PayVKP80KFA].name;
		mSupportedModels = QStringList() << mDeviceName;

		mPrinterModelId = CPayPrinters::Custom80;
		setDeviceParameter(CHardware::FR::PrinterModel, CPayPrinters::Models[mPrinterModelId].name);
	}
};

typedef PayVKP80FA<ShtrihOnlineFRBase<ShtrihTCPFRBase>> PayVKP80FATCP;
typedef PayVKP80FA<ShtrihOnlineFRBase<ShtrihSerialFRBase>> PayVKP80FASerial;

//--------------------------------------------------------------------------------
