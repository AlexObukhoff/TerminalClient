/* @file Данные моделей устройств на протоколе SSP. */

#pragma once

#include "SSPModelDataTypes.h"

//--------------------------------------------------------------------------------
namespace CSSP
{
	namespace Models
	{
		/// Название устройства по умолчанию.
		const char Default[] = "SSP cash acceptor";

		const char NV9ST[]         = "ITL NV9ST";
		const char NV10[]          = "ITL NV10";
		const char NV200ST[]       = "ITL NV200ST";
		const char NV200Spectral[] = "ITL NV200 Spectral";
		const char SH3ST[]         = "ITL SH3ST";
		const char SH4[]           = "ITL SH4";
		const char BV20ST[]        = "ITL BV20ST";
		const char BV50ST[]        = "ITL BV50ST";
		const char BV100[]         = "ITL BV100";

		class CData: public CSpecification<QString, SData>
		{
		public:
			CData()
			{
				add("NV0009", NV9ST,         3.78, 3.78);
				add("NV0010", NV10,          3.47, 3.47);
				add("NV200",  NV200ST,       4.49, 4.49);
				add("NVS200", NV200Spectral, 4.19, 4.17, true);
				add("SH0003", SH3ST,         6.54, 6.54);
				add("SH0004", SH4,           1.25, 1.25);
				add("BV0020", BV20ST,        4.21, 4.21);
				add("BV0050", BV50ST,        4.18, 4.18);
				add("BV0100", BV100,         4.18, 4.18);

				setDefault(SData(Default));
			}
		private:
			void add(const QString & aId, const QString & aModelName, double aFirmware, double aBaudrateFirmware, bool aVerified = false)
			{
				append(aId, SData(aModelName, aFirmware, aBaudrateFirmware, aVerified));
			}
		};

		static CData Data;
	}
}

//--------------------------------------------------------------------------------
