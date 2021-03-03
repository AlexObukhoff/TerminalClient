/* @file Данные моделей устройств на протоколе SSP. */

#pragma once

#include "Hardware/Common/ModelData.h"

//--------------------------------------------------------------------------------
namespace CSSP
{
	namespace Models
	{
		struct SData: public SBaseModelData
		{
			double lastFirmware;
			double baudrateFirmware;

			SData(): lastFirmware(0) {}
			SData(const QString & aName): SBaseModelData(aName, false, true), lastFirmware(0), baudrateFirmware(0) {}
			SData(const QString & aName, double aLastFirmware, double aBaudrateFirmware, bool aVerified = false): SBaseModelData(aName, aVerified, true),
				lastFirmware(aLastFirmware), baudrateFirmware(aBaudrateFirmware) {}
		};
	}
}

//--------------------------------------------------------------------------------
