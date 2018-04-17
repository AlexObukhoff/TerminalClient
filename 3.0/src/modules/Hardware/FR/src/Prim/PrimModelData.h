/* @file Данные моделей ФР ПРИМ. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSet>
#include <QtCore/QByteArray>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"
#include "Hardware/Printers/PrinterStatusCodes.h"

//--------------------------------------------------------------------------------
/// Константы PrimFR протокола.
namespace CPrimFR
{
	/// Модели.
	namespace Models
	{
		enum Enum
		{
			Unknown = 0,
			PRIM_07K,
			PRIM_08TK,
			PRIM_09TK,
			PRIM_21K_01,
			PRIM_21K_02,
			PRIM_21K_03,
			PRIM_88TK,
			AZIMUT_TMU950PK,

			OnlineUnknown = 100,
			PRIM_06F,
			PRIM_21FA
		};
	}

	typedef QSet<Models::Enum> TModels;

	inline TModels CommonModels()
	{
		return TModels()
			<< Models::Unknown
			<< Models::PRIM_07K
			<< Models::PRIM_08TK
			<< Models::PRIM_09TK
			<< Models::PRIM_88TK
			<< Models::AZIMUT_TMU950PK;
	}

	class CModelNames : public CSpecification<QByteArray, Models::Enum>
	{
	public:
		CModelNames()
		{
			append("\x8F\x90\x88\x8C\x2D\x30\x37\x8A",     Models::PRIM_07K);
			append("\x8F\x90\x88\x8C\x2D\x30\x38\x92\x8A", Models::PRIM_08TK);
			append("\x8F\x90\x88\x8C\x2D\x30\x39\x92\x8A", Models::PRIM_09TK);
			append("\x8F\x90\x88\x8C\x2D\x32\x31\x8A",     Models::PRIM_21K_02);
			append("\x8F\x90\x88\x8C\x2D\x38\x38\x92\x8A", Models::PRIM_88TK);
			append("\x8F\x90\x88\x8C\x2D\x38\x38\x92\x83", Models::AZIMUT_TMU950PK);

			// Online
			append("\x8F\x90\x88\x8C\x20\x30\x36\x2D\x94",     Models::PRIM_06F);
			append("\x8F\x90\x88\x8C\x20\x32\x31\x2D\x94\x80", Models::PRIM_21FA);
		}
	};

	static CModelNames ModelNames;

	//----------------------------------------------------------------------------	
	/// Параметры моделей.
	typedef QMap<int, int> TStatusBitShifts;
	typedef QMap<int, TStatusBitShifts> TStatusData;

	struct SModelParameters
	{
		QString name;
		bool hasBuffer;
		int feed;
		TStatusData statusData;
		bool verified;

		SModelParameters() : hasBuffer(false), feed(0), verified(false) {}
		SModelParameters(const QString & aName, bool aVerified, bool aHasBuffer, int aFeed, const TStatusData & aStatusData) :
			name(aName), hasBuffer(aHasBuffer), feed(aFeed), statusData(aStatusData), verified(aVerified) {}
	};

	/// Имя модели по умолчанию
	const char DefaultModelName[] = "PRIM FR";

	/// Имя модели по умолчанию
	const char DefaultOnlineModelName[] = "PRIM Online FR";

	/// Описатель данных моделей.
	class CModelData : public CSpecification<Models::Enum, SModelParameters>
	{
	public:
		CModelData()
		{
			// PRIM default
			TStatusData statusData;

			statusData[1][3] = PrinterStatusCode::Error::PrinterFR;

			statusData[2][2] = DeviceStatusCode::Error::CoverIsOpened;
			statusData[2][5] = PrinterStatusCode::Error::PaperEnd;
			statusData[2][6] = PrinterStatusCode::Error::PrinterFR;

			statusData[3][2] = DeviceStatusCode::Error::MechanismPosition;
			statusData[3][3] = PrinterStatusCode::Error::Cutter;
			statusData[3][5] = PrinterStatusCode::Error::PrinterFRCollapse;
			statusData[3][6] = PrinterStatusCode::Error::Temperature;

			statusData[4][3] = PrinterStatusCode::Warning::PaperNearEnd;
			statusData[4][6] = PrinterStatusCode::Error::PaperEnd;

			statusData[0][2] = DeviceStatusCode::Warning::OperationError;
			statusData[0][5] = PrinterStatusCode::Error::PrinterFR;

			// PRIM-07K
			TStatusData statusDataControlPaper(statusData);
			statusDataControlPaper[4][2] = PrinterStatusCode::Warning::ControlPaperNearEnd;
			statusDataControlPaper[4][5] = PrinterStatusCode::Error::ControlPaperEnd;

			// PRIM-21K Epson based
			TStatusData statusData21V0102(statusData);
			statusData21V0102[5][6] = PrinterStatusCode::Warning::PaperNearEnd;

			// PRIM-21K 03 Custom based
			TStatusData statusData21V03(statusData);
			statusData21V03[5][5] = PrinterStatusCode::OK::PaperInPresenter;
			//statusData21V03[5][6] = PrinterStatusCode::Warning::PaperEndVirtual;

			// Не онлайн
			data().insert(Models::PRIM_07K,    SModelParameters("Iskra PRIM-07K",    true,  false, 8, statusDataControlPaper));
			data().insert(Models::PRIM_08TK,   SModelParameters("Iskra PRIM-08TK",   true,  true,  4, statusData));
			data().insert(Models::PRIM_09TK,   SModelParameters("Iskra PRIM-09TK",   false, false, 4, statusData));
			data().insert(Models::PRIM_88TK,   SModelParameters("Iskra PRIM-88TK",   true,  false, 4, statusData));
			data().insert(Models::PRIM_21K_01, SModelParameters("Iskra PRIM-21K 01", true,  true,  5, statusData21V0102));
			data().insert(Models::PRIM_21K_02, SModelParameters("Iskra PRIM-21K 02", true,  true,  5, statusData21V0102));
			data().insert(Models::PRIM_21K_03, SModelParameters("Iskra PRIM-21K 03", true,  true,  0, statusData21V03));

			data().insert(Models::AZIMUT_TMU950PK,  SModelParameters("Iskra AZIMUT TM-U950PK", false, false, 4, statusDataControlPaper));

			// Онлайн
			data().insert(Models::PRIM_06F,  SModelParameters("Iskra PRIM 06-F",  true, true,  4, statusData));
			data().insert(Models::PRIM_21FA, SModelParameters("Iskra PRIM 21-FA", true, true,  0, statusData21V03));

			// default
			data().insert(Models::Unknown, SModelParameters(DefaultModelName, false, false, 8, statusData));
			data().insert(Models::OnlineUnknown, SModelParameters(DefaultOnlineModelName, false, true,  8, statusData));

			setDefault(SModelParameters("", false, true, 8, statusData));
		}
	};

	static CModelData ModelData;

	//----------------------------------------------------------------------------	
	/// Параметры моделей.
	inline QStringList getModelList(const TModels & aModels)
	{
		QStringList models;
		CModelData modelData;

		for (auto it = modelData.data().begin(); it != modelData.data().end(); ++it)
		{
			if (aModels.contains(it.key()) && (it->name != DefaultModelName))
			{
				models << it->name;
			}
		}

		return models;
	}
}

//--------------------------------------------------------------------------------
