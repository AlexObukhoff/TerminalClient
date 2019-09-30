/* @file Данные моделей Creator. */

#pragma once

#include "Hardware/Common/USBDeviceModelData.h"

//--------------------------------------------------------------------------------
namespace CCreatorReader
{
	/// Название кардридера Creator по умолчанию.
	const char DefaultName[] = "Unknown Creator cardreader";

	/// Данные модели.
	DECLARE_USB_MODEL(DetectingData, Creator, 0x0285, "CRT-288K");
}

//--------------------------------------------------------------------------------
