/* @file Данные моделей устройств на протоколе SSP. */

#pragma once

#include "Hardware/CashAcceptors/ModelData.h"

//--------------------------------------------------------------------------------
namespace CSSP
{
	class CModelData : public CSpecification<QString, SBaseModelData>
	{
	public:
		CModelData()
		{
			add("BV0020", "ITL BV20");
			add("BV0050", "ITL BV50");
			add("BV0100", "ITL BV100");
			add("NV0150", "ITL NV150");
			add("NV0200", "ITL NV200", true, false);
			add("NV0009", "ITL NV9 USB");
			add("NV0010", "ITL NV10 USB");
			add("SH0003", "ITL SH3");
			add("SH0004", "ITL SH4");
		}
	private:
		void add(const QString & aId, const QString & aModelName, bool aVerified = false, bool aUpdatable = false)
		{
			append(aId, SBaseModelData(aModelName, aVerified, aUpdatable));
		}
	};

	static CModelData ModelData;
}

//--------------------------------------------------------------------------------
