/* @file Базовый класс портов. */

// SDK
#include <SDK/Drivers/Components.h>

// Project
#include "IOPortBase.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
IOPortBase::IOPortBase() : mType(EPortTypes::Unknown), mDeviceIOLoging(ELoggingType::None), mOpeningTimeout(0)
{
}

//--------------------------------------------------------------------------------
void IOPortBase::setOpeningTimeout(int aTimeout)
{
	mOpeningTimeout = aTimeout;
	setConfigParameter(CHardware::Port::OpeningTimeout, aTimeout);
}

//--------------------------------------------------------------------------------
QString IOPortBase::getDeviceType()
{
	return CComponents::IOPort;
}

//--------------------------------------------------------------------------------
bool IOPortBase::clear()
{
	return true;
}

//--------------------------------------------------------------------------------
QString IOPortBase::getName() const
{
	return mSystemName;
}

//--------------------------------------------------------------------------------
bool IOPortBase::release()
{
	bool closingResult = close();
	bool result = MetaDevice::release();

	return closingResult && result;
}

//--------------------------------------------------------------------------------
EPortTypes::Enum IOPortBase::getType()
{
	return mType;
}

//--------------------------------------------------------------------------------
void IOPortBase::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	MetaDevice::setDeviceConfiguration(aConfiguration);

	if (aConfiguration.contains(CHardwareSDK::SystemName))
	{
		mSystemName = aConfiguration[CHardwareSDK::SystemName].toString();
	}

	if (aConfiguration.contains(CHardware::Port::DeviceModelName))
	{
		mConnectedDeviceName = aConfiguration[CHardware::Port::DeviceModelName].toString();
	}

	if (aConfiguration.contains(CHardware::Port::IOLogging))
	{
		mDeviceIOLoging = aConfiguration[CHardware::Port::IOLogging].value<ELoggingType::Enum>();
	}

	if (aConfiguration.contains(CHardware::Port::OpeningTimeout))
	{
		mOpeningTimeout = aConfiguration[CHardware::Port::OpeningTimeout].toInt();
	}
}

//--------------------------------------------------------------------------------
