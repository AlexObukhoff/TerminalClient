/* @file Данные моделей Creator. */

#pragma once

#include "Hardware/Common/USBDeviceModelData.h"

//--------------------------------------------------------------------------------
namespace CCreatorReader
{
	/// Название кардридера Creator по умолчанию.
	const char DefaultName[] = "Unknown Creator cardreader";

	/// Данные моделей.
	class ModelData : public CUSBDevice::CDetectingData
	{
	public:
		ModelData()
		{
			add(0x23d8, 0x0285, "Creator CRT-288K", true);
		}
	};
}

//--------------------------------------------------------------------------------
