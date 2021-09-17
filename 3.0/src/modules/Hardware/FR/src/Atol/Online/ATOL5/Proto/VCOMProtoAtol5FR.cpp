/* @file Прото-ФР на базе платформы АТОЛ5 для VCOM-порта. */

#pragma once

// Modules
#include "Hardware/Common/SerialDeviceUtils.h"

// Project
#include "VCOMProtoAtol5FR.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
bool ProtoAtol5FR<CInteractionTypes::ItExternalVCOM>::checkConnectionParameters(AtolDriverWrapper * aDriver, CAtol5OnlineFR::TConnectionParameters & aParametersChanged)
{
	if (!checkConnectionParameter(CHardwareSDK::SystemName))
	{
		return false;
	}

	QVariantMap requiredResourceParameters = getConfigParameter(CHardwareSDK::RequiredResourceParameters).toMap();
	QString portName = requiredResourceParameters.value(CHardwareSDK::SystemName).toString();

	EPortTypes::Enum portType = SerialDeviceUtils::getSystemData()[portName];

	if ((portType != EPortTypes::VirtualCOM) && (portType != EPortTypes::Unknown))
	{
		toLog(LogLevel::Error, QString("Port %1 is not VCOM type").arg(portName));
		return false;
	}

	aParametersChanged = CAtol5OnlineFR::TConnectionParameters()
		<< aDriver->checkSetting<int>(LIBFPTR_SETTING_PORT, LIBFPTR_PORT_COM)
		<< aDriver->checkSetting<QString>(LIBFPTR_SETTING_COM_FILE, portName);

	return true;
}

//--------------------------------------------------------------------------------
void ProtoAtol5FR<CInteractionTypes::ItExternalVCOM>::logConnectionParameters()
{
	QString portName = getConfigParameter(CHardwareSDK::RequiredResourceParameters).toMap().value(CHardwareSDK::SystemName).toString();
	toLog(LogLevel::Normal, "Port " + portName);
}

//--------------------------------------------------------------------------------
