/* @file Описатель типов данных портов. */

#pragma once

// SDK
#include <SDK/Drivers/IIOPort.h>

// Modules
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
class CPortTypeData: public CDescription<SDK::Driver::EPortTypes::Enum>
{
public:
	CPortTypeData()
	{
		using namespace SDK::Driver;

		append(EPortTypes::Unknown,     "Unknown");
		append(EPortTypes::COM,         "COM");
		append(EPortTypes::VirtualCOM,  "Virtual COM");
		append(EPortTypes::COMEmulator, "COM emulator");
		append(EPortTypes::USB,         "USB");
		append(EPortTypes::TCP,         "TCP");
	}

};

static CPortTypeData PortTypeData;

//--------------------------------------------------------------------------------
