/* @file Параметры моделей ФР Штрих. */

#pragma once

#include "ModelData.h"

//--------------------------------------------------------------------------------
namespace CShtrihFR { namespace Models {

CData::CData()
{
	/// Штрихи.
	addOld(0, "Shtrih-M Shtrih-FR-F");
	addOld(1, "Shtrih-M Shtrih-FR-F Kazakhstan");
	addOld(2, "ATOL Elves Mini-FR-F");
	addOld(3, "ATOL Felix-R-F");
	addOld(4, "Shtrih-M Shtrih-FR-K", true);
	addOld(5, "Shtrih-M Shtrih-950K");
	addOld(6, "Shtrih-M Elves-FR-K");
	addOld(7, "Shtrih-M Mini-FR-K", false, false, 4);
	addOld(8, "Shtrih-M Shtrih-FR-F Belorus");
	addOld(9, "Shtrih-M Combo-FR-K", true, false, 5);
	addOld(15, "Shtrih-M Kiosk-FR-K (Nippon)", false, true);
	addOld(17, "Shtrih-M NCR-001K");

	/// Новые модели, начиная с версии протокола 1.11.
	addOld(244, "Shtrih-M Kiosk-FR-K", false, true, 10);
	addOld(250, "Shtrih-M Shtrih-M-FR-K", true, false, 5);
	addOld(252, "Shtrih-M Light-FR-K", false, false, 4);

	/// Ярусы.
	addOld(243, "Shtrih-M Yarus-01K", true, true, 10, 0, QDate(2009, 7, 21), 40777);
	addOld(248, "Shtrih-M Yarus-02K", false, true, 10, 6, QDate(2009, 9, 15), 42633);

	/// Виртуальные ФР на старом протоколе.
	addOld(900, "NeoService", true);

	mDefaultFWDate = QDate(2017, 12, 29);

	/// Сторонние разработки.
	addNew( -1, "PAYONLINE",      "PayOnline-01-FA",     true, true,  1000, 0);    // определяется размером SD-карты, если она есть
	addNew( -2, "PAY VKP-80K-ФА", "PayVKP-80K-FA",       true, true,  1000, 0);    // определяется размером SD-карты, если она есть
	addNew(-30, "MSTAR-TK",       "Multisoft MStar-TK2", true, false, 0, 3, QDate(2017, 12, 14));

	/// Online.
	addNew(-31, "ШТРИХ-ON-LINE",  "Shtrih-M Shtrih-Online", true, false, 0, 3);
	addNew(-32, "ШТРИХ-ФР-01Ф",   "Shtrih-M Shtrih-FR-01F", true, false);
	addNew(-33, "ШТРИХ-ЛАЙТ-01Ф", "Shtrih-M Light-01F",     true, false, 0, 4, mDefaultFWDate, 20);
	addNew(-34, "ШТРИХ-ЛАЙТ-02Ф", "Shtrih-M Light-02F",     true, false, 0, 4, mDefaultFWDate, 20);
	addNew(-35, "ШТРИХ-М-01Ф",    "Shtrih-M Shtrih-M-01F",  true, false, 0, 5);
	addNew(-36, "ШТРИХ-М-02Ф",    "Shtrih-M Shtrih-M-02F",  true, false, 0, 5);
	addNew(-37, "ШТРИХ-МИНИ-01Ф", "Shtrih-M Mini-01F",      true, false, 0, 5);

	addNew(-51, "РР-04Ф",         "RR-Electro RR-04F",      true, false, 0, 3);

	setDefault(SModelData("", Models::Default, false, false, 0, 6, QDate::currentDate(), 0, 0));
}

//--------------------------------------------------------------------------------
QStringList CData::getNonEjectorModels(bool aOnline)
{
	QMap<int, CShtrihFR::SModelData> modelData = CShtrihFR::Models::CData().data();
	QStringList models;

	for (auto it = modelData.begin(); it != modelData.end(); ++it)
	{
		int id = it.key();
		CShtrihFR::SModelData & data = *it;

		if ((data.id.isEmpty() != aOnline) && !data.ejector &&
			(id != CShtrihFR::Models::ID::NeoService) &&
			(id != CShtrihFR::Models::ID::MStarTK2))
		{
			models.append(data.name);
		}
	}

	return models;
}

//--------------------------------------------------------------------------------
QStringList CData::getModelList(const TIds & aIds)
{
	QStringList models;

	foreach (int id, aIds)
	{
		models << value(id).name;
	}

	return models;
}

//--------------------------------------------------------------------------------
QStringList CData::getModelList(int aId)
{
	return getModelList(TIds() << aId);
}

//--------------------------------------------------------------------------------
void CData::addOld(int aNumber, const QString & aName, bool aVerified, bool aEjector, int aZReportQuantity, int aFeed, const QDate & aDate, int aBuild)
{
	append(aNumber, SModelData("", aName, aVerified, aEjector, aZReportQuantity, aFeed, aDate, aBuild, 0));
}

//--------------------------------------------------------------------------------
void CData::addNew(int aNumber, const char * aId, const QString & aName, bool aVerified, bool aEjector, int aZReportQuantity, int aFeed, const QDate & aDate, int aLinePrintingTimeout)
{
	QDate FWDate = aDate.isValid() ? aDate : (mDefaultFWDate.isValid() ? mDefaultFWDate : QDate::currentDate().addYears(100));
	append(aNumber, SModelData(QString::fromUtf8(aId), aName, aVerified, aEjector, aZReportQuantity, aFeed, FWDate, 0, aLinePrintingTimeout));
}

}}

//--------------------------------------------------------------------------------
