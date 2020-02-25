/* @file Данные моделей ФР на протоколе АТОЛ. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"

// Project
#include "AtolDataTypes.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний устройств на протоколе ATOL.
namespace CAtolFR
{
	/// Признак наличия ЭКЛЗ по имени устройства.
	const char EKLZPostfix = 'K';

	/// Признаки наличия ЭКЛЗ в имени устройства в ответе устройства.
	typedef QSet<char> TEKLZPostfixes;
	const TEKLZPostfixes EKLZPostfixes = TEKLZPostfixes() << EKLZPostfix << 'k' << '\x8A' << '\xAA';

	/// Актуальная версия прошивки для торговых ФР.
	const int OnlineTradeBuild = 5199;

	/// Актуальная версия прошивки для терминальных ФР.
	const int OnlineTerminalBuild = 5652;

	namespace Models
	{
		const char TriumF[]      = "ATOL Trium-F";
		const char FelixRF[]     = "ATOL Felix-R-F";
		const char Felix02K[]    = "ATOL Felix-02K";
		const char Mercury140F[] = "ATOL Mercury-140F";
		const char Tornado[]     = "ATOL Tornado";
		const char Mercury130[]  = "ATOL Mercury-130";
		const char MercuryMSK[]  = "ATOL Mercury-MSK";
		const char FelixRK[]     = "ATOL Felix-R-K";
		const char Felix3SK[]    = "ATOL Felix-3S-K";
		const char FPrint01K[]   = "ATOL FPrint-01K";
		const char FPrint02K[]   = "ATOL FPrint-02K";
		const char FPrint03K[]   = "ATOL FPrint-03K";
		const char FPrint88K[]   = "ATOL FPrint-88K";
		const char BIXOLON01K[]  = "ATOL BIXOLON-01K";
		const char MicroFR01K[]  = "ATOL Micro-FR-01K";
		const char FPrint5200K[] = "ATOL FPrint-5200K";
		const char Flaton11K[]   = "ATOL Flaton-11K";

		const char PayVKP80K[]   = "PayVKP-80K";
		const char PayPPU700K[]  = "PayPPU-700K";
		const char PayCTS2000K[] = "PayCTS-2000K";

		const char FPrint55K[]   = "ATOL FPrint-55K";
		const char FPrint11PTK[] = "ATOL FPrint-11PTK";
		const char FPrint22K[]   = "ATOL FPrint-22K";
		const char FPrint77PTK[] = "ATOL FPrint-77PTK";

		const char Atol11F[]     = "ATOL-11F";
		const char Atol15F[]     = "ATOL-15F";
		const char Atol25F[]     = "ATOL-25F";
		const char Atol30F[]     = "ATOL-30F";
		const char Atol42FC[]    = "ATOL-42FC";
		const char Atol52F[]     = "ATOL-52F";
		const char Atol55F[]     = "ATOL-55F";
		const char Atol77F[]     = "ATOL-77F";
		const char Atol91F[]     = "ATOL-91F";

		const char Paymaster[]   = "Sensis Kaznachej";
		const char FPrint22PTK[] = "ATOL FPrint-22PTK";
	}

	/// Данные моделей.
	typedef QPair<int, EFRType::Enum> TModelKey;

	class CModelData : public CSpecification<TModelKey, SModelData>
	{
	public:
		CModelData();

		/// Получить список моделей.
		QStringList getModelList(EFRType::Enum aFRType, bool aCanBeDP);

	private:
		/// Добавить кассовый ФР старых серий.
		void addOldTrade(int aModelId, int aMaxStringSize, QString aName, bool aCutter, bool aLineSpacing, bool aVerified = false, int aFeedingAmount = 0);

		/// Добавить кассовый ФР серии FPrint и/или следующих.
		void addTrade(int aModelId, int aMaxStringSize, QString aName, bool aCutter = true, bool aVerified = false, int aFeedingAmount = 0);

		/// Добавить терминальный ФР.
		void addTerminal(int aModelId, int aMaxStringSize, const QString & aName, int aBuild, bool aEjector, int aFeedingAmount = 0, int aZBufferSize = 0);

		/// Добавить онлайновый кассовый ФР.
		void addOnlineTrade(int aModelId, int aMaxStringSize, QString aName, bool aCutter, int aFeedingAmount = 0, int aBuild = OnlineTradeBuild, bool aVerified = true);

		/// Добавить онлайновый терминальный ФР.
		void addOnlineTerminal(int aModelId, int aMaxStringSize, const QString & aName, bool aEjector, int aFeedingAmount, int aZBufferSize = 0, int aBuild = OnlineTerminalBuild, bool aVerified = true);
	};
}

//--------------------------------------------------------------------------------
