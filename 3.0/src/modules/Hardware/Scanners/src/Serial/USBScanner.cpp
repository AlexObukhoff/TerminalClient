/* @file USB-сканер. */

#include "USBScanner.h"
#include "HHPModelData.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
USBScanner::USBScanner()
{
	mDeviceName = CUSBScanner::DefaultName;

	mDetectingData->append(CHHP::VendorID, CHHP::CModelData());
}

//--------------------------------------------------------------------------------
QStringList USBScanner::getModelList()
{
	QStringList models;

	foreach (CUSBDevice::SData data, CHHP::CModelData().data())
	{
		models << data.model;
	}

	return models;
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
