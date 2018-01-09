/* @file Данные моделей устройств на протоколе V2e. */

#pragma once

#include "Hardware/CashAcceptors/ModelData.h"

//--------------------------------------------------------------------------------
namespace CV2e
{
	namespace Models
	{
		const char Aurora[] = "GPT Aurora";
		const char Falcon[] = "GPT Falcon";
		const char Argus[]  = "GPT Argus";
	}

	typedef QSet<QString> TModelKeys;

	class ModelData : public CSpecification<TModelKeys, SBaseModelData>
	{
	public:
		ModelData();
		SBaseModelData getData(const QString & aKey);

	private:
		void add(const QString & aName, const TModelKeys & aModelKeys, bool aVerified = false);
	};
}

bool operator < (const CV2e::TModelKeys & aKeys1, const CV2e::TModelKeys & aKeys2);

//--------------------------------------------------------------------------------
