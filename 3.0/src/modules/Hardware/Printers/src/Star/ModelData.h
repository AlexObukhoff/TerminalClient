/* @file Данные моделей принтеров STAR. */

#pragma once

#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
/// Константы PrimFR протокола.
namespace CSTAR
{
	struct SModelData
	{
		bool verified;
		QString deviceId;
		double minFirmware;
		bool cutter;
		bool ejector;
		bool headThermistor;
		bool voltageSensor;
		bool innerPaperEndSensor;
		int feeding3;
		int feeding4;

		SModelData() : verified(false), minFirmware(0), cutter(true), ejector(false), headThermistor(false), voltageSensor(false), innerPaperEndSensor(true), feeding3(5), feeding4(4) {}
		SModelData(bool aVerified, const QString & aDeviceId, double aMinFirmware, bool aCutter, bool aEjector, bool aHeadThermistor, bool aVoltageSensor, bool aInnerPaperEndSensor, int aFeeding3, int aFeeding4) :
			verified(aVerified), deviceId(aDeviceId), minFirmware(aMinFirmware), cutter(aCutter), ejector(aEjector), headThermistor(aHeadThermistor),
			voltageSensor(aVoltageSensor), innerPaperEndSensor(aInnerPaperEndSensor), feeding3(aFeeding3), feeding4(aFeeding4) {}
	};

	/// Модели.
	namespace Models
	{
		#define ADD_LEXEME(aModel) const char aModel[] = "STAR "#aModel

		ADD_LEXEME(TUP542);
		ADD_LEXEME(TUP592);
		ADD_LEXEME(TUP942);
		ADD_LEXEME(TUP992);
		ADD_LEXEME(TSP613);
		ADD_LEXEME(TSP643);
		ADD_LEXEME(TSP651);
		ADD_LEXEME(TSP654);
		ADD_LEXEME(TSP654II);
		ADD_LEXEME(TSP743);
		ADD_LEXEME(TSP743II);
		ADD_LEXEME(TSP847);
		ADD_LEXEME(TSP847II);
		ADD_LEXEME(TSP828L);
		ADD_LEXEME(TSP1043);
		ADD_LEXEME(FVP10);

		const char Unknown[] = "Unknown STAR printer";
		const char UnknownEjector[] = "Unknown STAR printer with ejector";

		/// данные моделей.
		class CData : public CSpecification<QString, SModelData>
		{
		public:
			CData()
			{
				add(TUP542,   "TB5",      4.1, true,  true, false, true,  true);
				add(TUP592,   "TB5",      4.1, true,  true, true,  true,  true);
				add(TUP942,   "TB9",      5.1, true);
				add(TUP992,   "TB9",      5.1, true,  true, true,  false, false, true);
				add(TSP613,   "TSP600",   5.1, false, false);
				add(TSP643,   "TSP600",   5.1);
				add(TSP651,   "TSP650",   4.0, false, false);
				add(TSP654,   "TSP650",   4.0);
				add(TSP654II, "TSP650II", 2.0);
				add(TSP743,   "TSP700",   7.1);
				add(TSP743II, "TSP700II", 5.0, true, true, false, false, false, true, 5, 3);
				add(TSP847,   "TSP800",   7.3);
				add(TSP847II, "TSP800II", 2.0);
				add(TSP828L,  "TSP800L",  2.0);
				add(TSP1043,  "TSP1000",  3.0);
				add(FVP10,    "FVP10",    1.5);

				add(Unknown,        "", 0, false, true, false, false, false, false);
				add(UnknownEjector, "", 0, false, true, true,  false, false, true);
			}

		private:
			void add(const QString & aModel, const QString & aDeviceId, double aMinFirmware,
				bool aVerified = false, bool aCutter = true, bool aEjector = false, bool aHeadThermistor = false, bool aVoltageSensor = false, bool aInnerPaperEndSensor = true, int aFeeding3 = 5, int aFeeding4 = 4)
			{
				append(aModel, SModelData(aVerified, aDeviceId, aMinFirmware, aCutter, aEjector, aHeadThermistor, aVoltageSensor, aInnerPaperEndSensor, aFeeding3, aFeeding4));
			}
		};

		static CData Data;

		typedef QMap<QString, SModelData> TData;
	}
}

//--------------------------------------------------------------------------------
