/* @file Параметры моделей диспенсеров Puloon. */

#pragma once

// Modules
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CPuloonLCDM
{
	/// Параметры моделей.
	namespace Models
	{
		/// Данные модели.
		struct SData
		{
			QString name;
			bool verified;
			int ROMVersion;
			QString fullROMVersion;
			int units;

			SData(const QString & aName, bool aVerified, int aROMVersion, const QString & aFullROMVersion, int aUnits) :
				name(aName), verified(aVerified), ROMVersion(aROMVersion), fullROMVersion(aFullROMVersion), units(aUnits) {}
			SData() : verified(false), ROMVersion(0), units(0) {}
		};

		/// Данные моделей.
		class CData : public CSpecification<ushort, SData>
		{
		public:
			CData()
			{
				append(0xAD2C, SData("Puloon LCDM-1000", false, 16, "O16T", 1));
				append(0x8013, SData("Puloon LCDM-1000", false, 30, "O30T", 1));

				append(0x5624, SData("Puloon LCDM-2000", true,  20, "T20T", 2));
				append(0xB56D, SData("Puloon LCDM-2000", true,  30, "T30T", 2));

				setDefault(SData("Unknown Puloon dispenser", false, 0, "", 1));
			}
		};

		static CData Data;
	}
}

//--------------------------------------------------------------------------------
