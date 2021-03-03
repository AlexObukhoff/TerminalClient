/* @file Данные моделей устройств на протоколе CCNet. */

#pragma once

// Modules
#include "Hardware/Common/ModelData.h"

// Project
#include "Models.h"

//--------------------------------------------------------------------------------
namespace CCCNet
{
	class ModelData : public CSpecification<QString, SBaseModelData>
	{
	public:
		ModelData()
		{
			add("BB",  Models::CashcodeG200);

			add("GX",  Models::CashcodeGX, true);
			add("SM",  Models::CashcodeSM);
			add("SME", Models::CashcodeSM, true);
			add("MS",  Models::CashcodeMSM);
			add("VU",  Models::CashcodeMVU);
			add("FL",  Models::CashcodeMFL);
			add("SL",  Models::CashcodeSL);
			add("FLS", Models::CashcodeFLS);

			add("V77",    Models::ICTV77E);
			add("ICTL83", Models::ICTL83);
			add("ICTV77", Models::ICTV77E);

			add("BV009",  Models::ITLNV9);
			add("BV200",  Models::ITLNV200);

			add("C100",  Models::CreatorC100, true);
		}

	private:
		void add(const QString & aId, const QString & aName, bool aUpdatable = false)
		{
			append(aId, SBaseModelData(aName, true, aUpdatable));
		}
	};

	typedef QMap<QString, SBaseModelData>::iterator TModelDataIt;
}

//--------------------------------------------------------------------------------
