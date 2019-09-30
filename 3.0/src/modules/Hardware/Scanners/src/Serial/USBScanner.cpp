/* @file USB-сканер. */

#include "USBScanner.h"
#include "HHPDetectingData.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
USBScanner::USBScanner()
{
	mDeviceName = CHHP::DefaultName;

	mDetectingData->set(CUSBVendors::HHP, CHHP::DetectingData().data());
}

//--------------------------------------------------------------------------------
QStringList USBScanner::getModelList()
{
	return CHHP::DetectingData().getModelList(CUSBVendors::HHP);
}

//--------------------------------------------------------------------------------
bool USBScanner::getData(QByteArray & aAnswer)
{
	QVariantMap configuration;
	configuration.insert(CHardware::Port::MaxReadingSize, CUSBScanner::USBAnswerSize);
	mIOPort->setDeviceConfiguration(configuration);

	return TUSBScanner::getData(aAnswer);
}

//--------------------------------------------------------------------------------
