/* @file Параметры моделей ФР Штрих. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDate>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"

// Project
#include "ShtrihFRDataTypes.h"

//--------------------------------------------------------------------------------
namespace CShtrihFR
{
	typedef QList<int> TIds;

	/// Параметры моделей.
	namespace Models
	{
		/// Модели по умолчанию.
		const char Default[] = "Shtrih FR";
		const char RetractorDefault[] = "Shtrih retractor FR";
		const char EjectorDefault[] = "Shtrih ejector FR";
		const char OnlineDefault[] = "Shtrih online FR";
		const char OnlineRetractorDefault[] = "Shtrih retractor online FR";

		/// ИД моделей.
		namespace ID
		{
			const int NoModel           = 0x7FFF;

			/// Старые.
			const int ShtrihFRF         = 0;
			const int ShtrihFRFKazah    = 1;
			const int ATOLElvesMiniFRF  = 2;
			const int ATOLFelixRF       = 3;
			const int ShtrihFRK         = 4;
			const int Shtrih950K        = 5;
			const int ShtrihElvesFRK    = 6;
			const int ShtrihMiniFRK     = 7;
			const int ShtrihFRFBelorus  = 8;
			const int ShtrihComboFRK    = 9;
			const int ShtrihKioskFRK    = 15;
			const int ShtrihNCR001K     = 17;

			const int Shtrih500         = 0;

			const int Yarus01K          = 243;
			const int Yarus02K          = 248;

			const int ShtrihKioskFRK_2  = 244;
			const int ShtrihMFRK        = 250;
			const int ShtrihLightFRK    = 252;

			/// Виртуальные.
			const int NeoService        = 900;

			/// Онлайновые.
			const int PayOnline01FA     = -1;
			const int PayVKP80KFA       = -2;
			const int MStarTK2          = -30;
			const int ShtrihOnline      = -31;
			const int ShtrihFR01F       = -32;
			const int ShtrihLight01F    = -33;
			const int ShtrihLight02F    = -34;
			const int ShtrihM01F        = -35;
			const int ShtrihM02F        = -36;
			const int ShtrihMini01F     = -37;

			const int RRElectro03F      = -51;
			const int RRElectro04F      = -52;
		}

		/// Данные моделей.
		class CData : public CSpecification<int, SModelData>
		{
		public:
			CData();

			QStringList getNonEjectorModels(bool aOnline);
			QStringList getModelList(const TIds & aIds);
			QStringList getModelList(int aId);

		private:
			void addOld(int aNumber, const QString & aName, bool aVerified = false, bool aEjector = false, int aZReportQuantity = 0, int aFeed = 6,
				const QDate & aDate = QDate::currentDate().addYears(100), int aBuild = 0);

			void addNew(int aNumber, const char * aId, const QString & aName, bool aVerified = false, bool aEjector = false, int aZReportQuantity = 0, int aFeed = 6,
				const QDate & aDate = QDate(), int aLinePrintingTimeout = 0);

			QDate mDefaultFWDate;
		};
	}

	#define ADD_OLD_SHTRIH_FIELDS(aModel, aFields) append(Models::ID::aModel, SFields(TFields() << aFields));
	#define ADD_NEW_SHTRIH_FIELDS(aModel, aFields, aTables) append(Models::ID::aModel, SFields(TFields() << aFields, TTables() << aTables));

	/// Параметры ФР.
	namespace FRParameters
	{
		class CFields: public CSpecification<int, SFields>
		{
		public:
			CFields()//                                 0    1     2     3     4    5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20
			{
				ADD_OLD_SHTRIH_FIELDS(ShtrihFRK,        2 << 4 <<  3 << 36 <<  6 << 8 << 20 << 21 << 25 << 15 << 40 << 17 << 19 << 44 << 46 << 26);
				ADD_OLD_SHTRIH_FIELDS(ShtrihMiniFRK,    2 << 3 << NA << NA <<  5 << 7 << 18 << 19 << 30 << 32 << 35 << 15 << 17 << 40 << 42 << 33);
				ADD_OLD_SHTRIH_FIELDS(ShtrihComboFRK,   2 << 3 << NA << NA <<  5 << 7 << 18 << 19 << 22 << 14 << 32 << 15 << 17 << NA << NA << 23);

				ADD_OLD_SHTRIH_FIELDS(Yarus01K,         2 << 3 << NA << NA <<  5 << 6 << 12 << 13 << NA <<  8 << 23 <<  9 << 11 << NA << 27 << NA << 25 << 26 << 28);
				ADD_OLD_SHTRIH_FIELDS(Yarus02K,         2 << 3 << NA << NA <<  5 << 6 << 12 << 13 << NA <<  8 << 23 <<  9 << 11 << NA << NA << NA << 25 << 26);

				ADD_OLD_SHTRIH_FIELDS(ShtrihMFRK,       2 << 3 << NA << NA <<  5 << 7 << 17 << 18 << 21 << 13 << 30 << 14 << 16 << NA << NA << 22);
				ADD_OLD_SHTRIH_FIELDS(ShtrihLightFRK,   2 << 3 << NA << NA <<  5 << 7 << 17 << 18 << 21 << 13 << 30 << 14 << 16 << NA << NA << 22);
				ADD_OLD_SHTRIH_FIELDS(ShtrihKioskFRK_2, 2 << 3 << NA << NA <<  5 << 6 << 12 << 13 << NA <<  8 << 23 <<  9 << 11 << NA << 27 << NA << 25 << 26);

				// NeoService
				ADD_OLD_SHTRIH_FIELDS(NeoService,       2 << 4 <<  3 << NA <<  6 << 8 << 20 << 21 << 25 << 15 << 39 << 17 << 19 << 41 << 43 << 26);

				//TODO: клише 39?
				ADD_NEW_SHTRIH_FIELDS(PayOnline01FA,    2 << 3 << NA << NA << NA << 7 << 17 << NA << 21 << NA << NA << 14 << 16 << NA << 39 << 22 <<  3 <<  4 <<  5 <<  6 <<  7,
				                                        1 << 1 <<  1 <<  1 <<  1 << 1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 << 24 << 24 << 24 << 24 << 24);
				ADD_NEW_SHTRIH_FIELDS(PayVKP80KFA,      2 << 3 << NA << NA << NA << 7 << 17 << NA << 21 << NA << NA << 14 << 16 << NA << 39 << 22 <<  3 <<  4 <<  5 <<  6 <<  7,
				                                        1 << 1 <<  1 <<  1 <<  1 << 1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 << 24 << 24 << 24 << 24 << 24);
				ADD_NEW_SHTRIH_FIELDS(MStarTK2,         2 << 3 << NA << NA <<  5 << 7 << 17 << 18 << 21 << NA << 30 << 14 << 16 << NA << NA << 22 <<  3 <<  4 <<  5 <<  6 <<  7,
				                                        1 << 1 <<  1 <<  1 <<  1 << 1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 << 24 << 24 << 24 << 24 << 24);
				ADD_OLD_SHTRIH_FIELDS(ShtrihOnline,     2 << 3 << NA << NA << NA << 7 << 17 << NA << 21 << 13 << 30 << 14 << 16 << NA << 39 << 22);
				ADD_OLD_SHTRIH_FIELDS(ShtrihFR01F,      2 << 3 << NA << NA << NA << 7 << 17 << 18 << 21 << 13 << 30 << 14 << 16 << NA << 39 << 22);
				ADD_OLD_SHTRIH_FIELDS(ShtrihLight01F,   2 << 3 << NA << NA << NA << 7 << 17 << 18 << 21 << 13 << 30 << 14 << 16 << NA << 45 << 28);
				ADD_OLD_SHTRIH_FIELDS(ShtrihLight02F,   2 << 3 << NA << NA << NA << 7 << 17 << 18 << 21 << 13 << 30 << 14 << 16 << NA << 45 << 28);
				ADD_OLD_SHTRIH_FIELDS(ShtrihM01F,       2 << 3 << NA << NA << NA << 7 << 17 << 18 << 21 << 13 << 30 << 14 << 16 << NA << 39 << 22);
				ADD_OLD_SHTRIH_FIELDS(ShtrihM02F,       2 << 3 << NA << NA << NA << 7 << 17 << 18 << 21 << 13 << 30 << 14 << 16 << NA << 39 << 22);
				ADD_OLD_SHTRIH_FIELDS(ShtrihMini01F,    2 << 3 << NA << NA << NA << 7 << 17 << 18 << 21 << 13 << 30 << 14 << 16 << NA << 39 << 22);

				ADD_OLD_SHTRIH_FIELDS(RRElectro03F,     2 << 3 << NA << NA << NA << 7 << 17 << NA << 21 << 13 << 30 << 14 << 16 << NA << 39 << 22);
				ADD_OLD_SHTRIH_FIELDS(RRElectro04F,     2 << 3 << NA << NA << NA << 7 << 17 << NA << 21 << 13 << 30 << 14 << 16 << NA << 39 << 22);

				//                                      0    1     2     3     4    5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20
			}
		};

		static CFields Fields;
	}
}

//--------------------------------------------------------------------------------
