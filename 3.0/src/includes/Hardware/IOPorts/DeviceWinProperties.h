/* @file Описатель свойств устройств windows. */

#pragma once

// Windows
#include <windows.h>
#include <setupapi.h>

// Modules
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CDeviceWinProperties
{
	/// Префикс системного win-параметра.
	const char Prefix[] = "SPDRP";
}

//--------------------------------------------------------------------------------
class DeviceWinProperties : public CDescription<DWORD>
{
public:
	DeviceWinProperties()
	{
		APPEND(SPDRP_CONFIGFLAGS);
		APPEND(SPDRP_EXCLUSIVE);
		APPEND(SPDRP_FRIENDLYNAME);
		APPEND(SPDRP_LOCATION_INFORMATION);
		APPEND(SPDRP_LOWERFILTERS);
		APPEND(SPDRP_REMOVAL_POLICY_OVERRIDE);
		APPEND(SPDRP_SECURITY);
		APPEND(SPDRP_SECURITY_SDS);
		APPEND(SPDRP_UI_NUMBER_DESC_FORMAT);
		APPEND(SPDRP_UPPERFILTERS);
		APPEND(SPDRP_ADDRESS);
		APPEND(SPDRP_BUSNUMBER);
		APPEND(SPDRP_BUSTYPEGUID);
		APPEND(SPDRP_CHARACTERISTICS);
		APPEND(SPDRP_COMPATIBLEIDS);
		APPEND(SPDRP_CAPABILITIES);
		APPEND(SPDRP_CLASS);
		APPEND(SPDRP_CLASSGUID);
		APPEND(SPDRP_DEVICE_POWER_DATA);
		APPEND(SPDRP_DEVICEDESC);
		APPEND(SPDRP_DEVTYPE);
		APPEND(SPDRP_DRIVER);
		APPEND(SPDRP_ENUMERATOR_NAME);
		APPEND(SPDRP_HARDWAREID);
		APPEND(SPDRP_INSTALL_STATE);
		APPEND(SPDRP_LEGACYBUSTYPE);
		APPEND(SPDRP_MFG);
		APPEND(SPDRP_PHYSICAL_DEVICE_OBJECT_NAME);
		APPEND(SPDRP_REMOVAL_POLICY);
		APPEND(SPDRP_REMOVAL_POLICY_HW_DEFAULT);
		APPEND(SPDRP_SERVICE);
		APPEND(SPDRP_UI_NUMBER);
		APPEND(SPDRP_LOCATION_PATHS);
	}
};

//--------------------------------------------------------------------------------
