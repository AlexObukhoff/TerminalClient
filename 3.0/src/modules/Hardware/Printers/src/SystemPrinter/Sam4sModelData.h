/* @file Данные моделей принтеров Sam4s. */

#pragma once

#include "Hardware/Common/ModelData.h"
#include "Hardware/Printers/Sam4sModels.h"

//--------------------------------------------------------------------------------
/// Константы принтеров Sam4s.
namespace CSam4s
{
	/// Модели.
	namespace Models
	{
		/// данные моделей.
		class CData : public CBaseModelData<QString>
		{
		public:
			CData()
			{
				add("ELLIX40", Ellix40);
				add("ELLIX50", Ellix50, true);

				setDefault(SBaseModelData(Unknown));
			}
		};

		static CData Data;
	}
}

//--------------------------------------------------------------------------------
