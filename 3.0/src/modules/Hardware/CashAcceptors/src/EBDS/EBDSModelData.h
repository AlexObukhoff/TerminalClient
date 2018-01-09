/* @file Данные моделей устройств на протоколе EBDS. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QPair>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/CashAcceptors/ModelData.h"

//--------------------------------------------------------------------------------
namespace CEBDS
{
	typedef QPair<char, bool> TModelData;

	class ModelData : public CSpecification<TModelData, SBaseModelData>
	{
	public:
		ModelData()
		{
			add(0x01, "MEI ZT1000, USA");
			add(0x11, "MEI ZT1100, USA");
			add(0x0C, "MEI ZT1107, USA");
			add(0x0F, "MEI ZT1200, Australia");
			add(0x16, "MEI ZT1200, Canada");
			add(0x17, "MEI ZT1200, USA");

			add(0x41, "MEI AE2600 Gen2D, Australia)");
			add(0x42, "MEI AE2800 Gen2D, Russia)");
			add(0x43, "MEI AE2600 Gen2D, Canada)");
			add(0x43, "MEI AE2800 Gen2D, Europe)");
			add(0x45, "MEI VN2300 Gen1, USA Economy");
			add(0x46, "MEI VN2600 Gen2B & 2D, China");
			add(0x46, "MEI AE2800 Gen2D, Argentina");
			add(0x48, "MEI AE24V Gen1, USA Economy");
			add(0x4D, "MEI AE2800 Gen2D, Mexico");
			add(0x50, "MEI AE2600 Gen2B, C & D, USA Premium");
			add(0x51, "MEI 2000 Series, Philippine");
			add(0x52, "MEI VN2500 Gen1, USA");
			add(0x56, "MEI VN2500 Gen2, USA");
			add(0x57, "MEI AE2800 Gen2D, Brazil");
			add(0x58, "MEI AE2800 Gen2D, USA Expanded");

			add(0x1E, "MEI 3000 BDS, USA");
			add(0x1F, "MEI 3000 EBDS, USA");

			add(0x4A, "MEI CASHFL0W SC 66, Fixed Width");
			add(0x54, "MEI CASHFL0W SC 83", true);
			add(0x55, "MEI CASHFL0W SC 66, Extended Notes");

			add(0x4A, "MEI CASHFL0W SCN 66, Fixed Width (ADVANCED)", false, true);
			add(0x54, "MEI CASHFL0W SCN 83 (ADVANCED)", true, true);
			add(0x55, "MEI CASHFL0W SCN 66, Extended Notes (ADVANCED)", false, true);
		}

	private:
		void add(char aCode, const QString & aName, bool aVerified = false, bool aAdvanced = false)
		{
			append(TModelData(aCode, aAdvanced), SBaseModelData(aName, aVerified));
		}
	};
}

//--------------------------------------------------------------------------------
