/* @file Дефолтное HID-устройство на COM-порту. */

#pragma once

#include "SerialScanner.h"

//--------------------------------------------------------------------------------
SerialScanner::SerialScanner()
{
	mDeviceName = "Generic serial HID";
	mAutoDetectable = false;
}

//--------------------------------------------------------------------------------
bool SerialScanner::getData(QByteArray & aAnswer)
{
	if (mIOPort->getType() == SDK::Driver::EPortTypes::VirtualCOM)
	{
		QVariantMap configuration;
		configuration.insert(CHardware::Port::COM::WaitResult, true);
		mIOPort->setDeviceConfiguration(configuration);

		return mIOPort->read(aAnswer, CScanner::PollingInterval);
	}

	return TSerialScanner::getData(aAnswer);
}

//--------------------------------------------------------------------------------
