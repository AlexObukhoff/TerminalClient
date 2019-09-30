/* @file Параметры моделей ФР семейства MStar на протоколе AFP. */

#pragma once

// Modules
#include "Hardware/Common/Specifications.h"

// Project
#include "AFPFRDataTypes.h"

//--------------------------------------------------------------------------------
namespace CAFPFR
{
	typedef QList<int> TIds;

	/// Параметры моделей.
	namespace Models
	{
		/// Данные моделей.
		class CData: public CSpecification<QString, Models::SData>
		{
		public:
			CData()
			{
				add("MSTAR-TK", "Multisoft MStar-TK2", "4.4.4.0", false);
			}

		private:
			void add(const QString & aId, const QString & aName, const QString & aFirmware, bool aVerified = true)
			{
				append(aId, Models::SData(aName, aFirmware, aVerified));
			}
		};

		static CData Data;
	}
}

//--------------------------------------------------------------------------------
