/* @file Данные моделей устройств на протоколе SSP. */

#pragma once

#include "Hardware/CashAcceptors/ModelData.h"

//--------------------------------------------------------------------------------
namespace CSSP
{
	namespace Models
	{
		/// Название устройства по умолчанию.
		const char Default[] = "SSP cash acceptor";

		const char NV9[]   = "ITL NV9";
		const char NV10[]  = "ITL NV10";
		const char NV150[] = "ITL NV150";
		const char NV200[] = "ITL NV200";
		const char NV200Spectral[] = "ITL NV200 Spectral";
		const char SH3[]   = "ITL SH3";
		const char SH4[]   = "ITL SH4";
		const char BV20[]  = "ITL BV20";
		const char BV50[]  = "ITL BV50";
		const char BV100[] = "ITL BV100";

		class CData: public CSpecification<QString, SBaseModelData>
		{
		public:
			CData()
			{
				add("NV0009", NV9);
				add("NV0010", NV10);
				add("NV0150", NV150);
				add("NV200",  NV200);
				add("NVS200", NV200Spectral, true);
				add("SH0003", SH3);
				add("SH0004", SH4);
				add("BV0020", BV20);
				add("BV0050", BV50);
				add("BV0100", BV100);

				setDefault(SBaseModelData(Default));
			}
		private:
			void add(const QString & aId, const QString & aModelName, bool aVerified = false)
			{
				append(aId, SBaseModelData(aModelName, aVerified, true));
			}
		};

		static CData Data;
	}
}

//--------------------------------------------------------------------------------
