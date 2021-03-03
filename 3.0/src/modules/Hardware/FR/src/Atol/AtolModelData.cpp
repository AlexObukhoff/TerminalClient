/* @file Данные моделей ФР на протоколе АТОЛ. */

#include "AtolModelData.h"

using namespace CAtolFR;

//--------------------------------------------------------------------------------
CModelData::CModelData()
{                                         // Cutter LineSpacing
	addOldTrade(13, 40, Models::TriumF,      true,  false);
	addOldTrade(14, 20, Models::FelixRF,     false, true);
	addOldTrade(15, 20, Models::Felix02K,    false, true);
	addOldTrade(16, 24, Models::Mercury140F, false, false);
	addOldTrade(20, 40, Models::Tornado,     true,  true);
	addOldTrade(22,  0, Models::Mercury130,  true,  false);
	addOldTrade(23, 39, Models::MercuryMSK,  true,  true);
	addOldTrade(24, 32, Models::FelixRK,     false, true);
	addOldTrade(27, 32, Models::Felix3SK,    false, true);
	addOldTrade(33, 42, Models::BIXOLON01K,  true,  false);
	addOldTrade(34,  0, Models::MicroFR01K,  true,  false);
	addOldTrade(36,  0, Models::Flaton11K,   true,  false);

	                                //   Cutter  Verif Feed
	addTrade(29, 36, Models::FPrint01K);
	addTrade(30, 40, Models::FPrint02K,   true,  true,  3);
	addTrade(31, 32, Models::FPrint03K,   false);
	addTrade(32, 42, Models::FPrint88K);
	addTrade(35, 36, Models::FPrint5200K, true,  true,  4);
	addTrade(47, 36, Models::FPrint55K,   true,  true,  6);
	addTrade(51, 36, Models::FPrint11PTK, false, true,  3);
	addTrade(52, 48, Models::FPrint22K,   true,  true,  6);
	addTrade(53, 48, Models::FPrint77PTK, true,  true,  6);

	/// Утаревшие модели, нет Z-буфера
	addTerminal(37, 41, Models::PayVKP80K,   5477, true);
	addTerminal(38, 42, Models::PayPPU700K,  5768, true);

	/// Eсть Z-буффер.
	addTerminal(41, 41, Models::PayVKP80K,   1149, true,  0, 25);
	addTerminal(45, 42, Models::PayPPU700K,  1172, true,  0, 25);
	addTerminal(39, 48, Models::PayCTS2000K, 1062, false, 4, 12);
	addTerminal(46, 48, Models::PayCTS2000K, 1062, false, 4, 12);

	/// Онлайн.
	addOnlineTrade(67, 32, Models::Atol11F, false, 3);
	addOnlineTrade(78, 32, Models::Atol15F, false, 3);
	addOnlineTrade(57, 48, Models::Atol25F, true,  3);
	addOnlineTrade(61, 32, Models::Atol30F, false, 3);
	addOnlineTrade(64, 36, Models::Atol52F, true,  6, CAtolFR::OnlineTradeBuild, false);
	addOnlineTrade(62, 36, Models::Atol55F, true,  6);
	addOnlineTrade(69, 48, Models::Atol77F, true,  6, CAtolFR::OnlineTradeBuild, false);
	addOnlineTrade(82, 32, Models::Atol91F, true,  3, CAtolFR::OnlineTradeBuild, false);

	addOnlineTrade(63, 48, Models::FPrint22PTK, true, 6, 7651);

	addOnlineTerminal(76, 67, Models::Paymaster, true, 0, 32, 7942);
	addOnlineTerminal(77, 32, Models::Atol42FC,  true, 0, 32, CAtolFR::OnlineTerminalBuild, false);

	setDefault(SModelData(0, 32, "ATOL device", false, false, false, EFRType::FS, 0, false, true, 0, 0, true));
}

//--------------------------------------------------------------------------------
QStringList CModelData::getModelList(EFRType::Enum aFRType, bool aCanBeDP)
{
	QSet<QString> models;

	foreach (const SModelData & modelData, data())
	{
		if (!modelData.terminal && (modelData.FRType == aFRType) && (modelData.canBeDP == aCanBeDP))
		{
			models.insert(modelData.name);
		}
	}

	return models.toList();
}

//--------------------------------------------------------------------------------
void CModelData::addOldTrade(int aModelId, int aMaxStringSize, QString aName, bool aCutter, bool aLineSpacing, bool aVerified, int aFeedingAmount)
{
	EFRType::Enum type = aName.endsWith(EKLZPostfix) ? EFRType::EKLZ : EFRType::NoEKLZ;
	append(TModelKey(aModelId, EFRType::EKLZ), SModelData(aModelId, aMaxStringSize, aName, false, false, aVerified, type, 0, false, aCutter, aFeedingAmount, 0, aLineSpacing));
}

//--------------------------------------------------------------------------------
void CModelData::addTrade(int aModelId, int aMaxStringSize, QString aName, bool aCutter, bool aVerified, int aFeedingAmount)
{
	EFRType::Enum type = aName.endsWith(EKLZPostfix) ? EFRType::EKLZ : EFRType::NoEKLZ;
	append(TModelKey(aModelId, EFRType::EKLZ), SModelData(aModelId, aMaxStringSize, aName, true, false, aVerified, type, 0, false, aCutter, aFeedingAmount, 0, true));

	if (type == EFRType::EKLZ)
	{
		aName.chop(1);
		append(TModelKey(aModelId, EFRType::NoEKLZ), SModelData(aModelId, aMaxStringSize, aName, true, false, aVerified, EFRType::NoEKLZ, 0, false, aCutter, aFeedingAmount, 0, true));
	}
}

//--------------------------------------------------------------------------------
void CModelData::addTerminal(int aModelId, int aMaxStringSize, const QString & aName, int aBuild, bool aEjector, int aFeedingAmount, int aZBufferSize)
{
	append(TModelKey(aModelId, EFRType::EKLZ), SModelData(aModelId, aMaxStringSize, aName, false, true, true, EFRType::EKLZ, aBuild, aEjector, true, aFeedingAmount, aZBufferSize, true));
}

//--------------------------------------------------------------------------------
void CModelData::addOnlineTrade(int aModelId, int aMaxStringSize, QString aName, bool aCutter, int aFeedingAmount, int aBuild, bool aVerified)
{
	append(TModelKey(aModelId, EFRType::FS), SModelData(aModelId, aMaxStringSize, aName, false, false, aVerified, EFRType::FS, aBuild, false, aCutter, aFeedingAmount, 0, true));
}

//--------------------------------------------------------------------------------
void CModelData::addOnlineTerminal(int aModelId, int aMaxStringSize, const QString & aName, bool aEjector, int aFeedingAmount, int aZBufferSize, int aBuild, bool aVerified)
{
	append(TModelKey(aModelId, EFRType::FS), SModelData(aModelId, aMaxStringSize, aName, false, true, aVerified, EFRType::FS, aBuild, aEjector, true, aFeedingAmount, aZBufferSize, true));
}

//--------------------------------------------------------------------------------
