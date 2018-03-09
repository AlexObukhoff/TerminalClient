/* @file Данные моделей устройств на протоколе ccTalk. */

#pragma once

// STL
#include <algorithm>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <Common/QtHeadersEnd.h>

// Project
#include "CCTalkModelData.h"
#include "CCTalkCoinAcceptorConstants.h"

//--------------------------------------------------------------------------------
CCCTalk::CVendorData::CVendorData()
{
	add("AES", "Aardvark Embedded Solutions Ltd", "AES");
	add("ALB", "Alberici");
	add("ANI", "AlfaNet informatika d.o.o", "AlfaNet");
	add("AST", "AstroSystems Ltd", "AstroSystems");
	add("AZK", "Azkoyen");
	add("CMG", "Comestero Group", "Comestero");
	add("CCC", "Crane CashCode Company", "CashCode");
	add("ECP", "Encopim SL");
	add("GTD", "Gaming Technology Distribution", "GTD");
	add("HIM", "Himecs");
	add("ITL", "Innovative Technology Ltd", "ITL");
	add("INT", "Intergrated Technology Ltd", "INT");
	add("ICT", "International Currency Technologies Corp.", "ICT");
	add("JCM", "Japan Cash Machine Ltd", "JCM");
	add("MEI", "Mars Electronics International", "MEI");
	add("MSC", "Microsystem Controls Pty. Ltd. (Microcoin)", "Microcoin", true);
	add("MCI", "Money Controls (International)", "Money Controls");
	add("NRI", "National Rejectors Inc", "NRI");
	add("PMD", "Phoenix Mecano Digital Elektronik GmbH", "Phoenix Mecano");
	add("SEL", "Starpoint Electrics Ltd", "Starpoint");
	add("WHM", "WH Munzprufer");
}

//--------------------------------------------------------------------------------
void CCCTalk::CVendorData::add(const QString & aVendorID, const QString & aVendor, const QString & aShortVendor, bool aComplexEnabling)
{
	append(aVendorID, SVendorData(aVendor, aShortVendor, aComplexEnabling));
}

//--------------------------------------------------------------------------------
CCCTalk::CModelData::CModelData()
{
	add("MSC", "SP2", 6, 13);
	data()["MSC"]["SP2"].minVersions.insert(Currency::RUB, 5.38);

	add("MSC", "SP3", 6, 13);
	data()["MSC"]["SP3"].minVersions.insert(Currency::RUB, 5.42);

	add("NRI", "G13");
	data()["NRI"]["G13"].minVersions.insert(Currency::RUB, 52.03);
	data()["NRI"]["G13"].minVersions.insert(Currency::KZT, 52.05);
	data()["NRI"]["G13"].unsupported = CCCTalk::TUnsupported()
		<< CCCTalk::Command::BaseYear
		<< CCCTalk::Command::CreationDate
		<< CCCTalk::Command::SoftLastDate;

	add("NRI", "BV", 0, 0, "Pelicano");
	data()["NRI"]["BV"].unsupported = CCCTalk::TUnsupported()
		<< CCCTalk::Command::BaseYear
		<< CCCTalk::Command::CreationDate
		<< CCCTalk::Command::SoftLastDate;

	add("ICT", "SCA1");
	add("WHM", "EMP", 0, 0, "EMP-800");
}

//--------------------------------------------------------------------------------
void CCCTalk::CModelData::add(const QString & aVendorId, const QString & aModelId, uchar aFault, uchar aError, const QString & aModel, bool aVerified)
{
	data()[aVendorId].insert(aModelId, SModelData(aFault, aError, aModel, aVerified));
}

//--------------------------------------------------------------------------------
QString CCCTalk::CVendorData::getName(const QString & aId)
{
	QString vendorID = aId.simplified().toUpper();
	SVendorData & vendorData = data()[vendorID];

	return vendorData.shortVendor.isEmpty() ? vendorData.vendor : vendorData.shortVendor;
}

//--------------------------------------------------------------------------------
QString CCCTalk::getModelData(const QByteArray & aVendorID, const QByteArray & aModelID, CCCTalk::SModelData & aModelData)
{
	QString vendorID = aVendorID.simplified().toUpper();

	if (!VendorData.data().contains(vendorID))
	{
		return DefaultDeviceName;
	}

	QString vendor = VendorData.getName(vendorID);
	QString modelId = QString(aModelID).remove(QRegExp("[ \\-]+")).toLatin1();
	//TModelData modelData = ModelData[vendorID];

	for (TModelDataIt it = ModelData.data().begin(); it != ModelData.data().end(); ++it)
	{
		for (TModelData::iterator jt = it->begin(); jt != it->end(); ++jt)
		{
			if (modelId.indexOf(jt.key()) != -1)
			{
				aModelData = jt.value();
				QString model = jt->model;

				if (model.isEmpty())
				{
					model = jt.key();
				}

				return QString("%1 %2").arg(vendor).arg(model);
			}
		}
	}

	return QString("%1 %2").arg(vendor).arg(DeviceNamePostfix);
}

//--------------------------------------------------------------------------------
QStringList CCCTalk::getModels(bool aComplexEnabling)
{
	QStringList result;
	CVendorData vendorData;
	CModelData modelData;

	for (TVendorDataIt it = vendorData.data().begin(); it != vendorData.data().end(); ++it)
	{
		if (it->complexEnabling == aComplexEnabling)
		{
			QString vendorId = it.key();
			QString vendor = vendorData.getName(vendorId);
			result << QString("%1 %2").arg(vendor).arg(DeviceNamePostfix);

			for (auto jt = modelData[vendorId].begin(); jt != modelData[vendorId].end(); ++jt)
			{
				QString model = jt->model;

				if (model.isEmpty())
				{
					model = jt.key();
				}

				result << QString("%1 %2").arg(vendor).arg(model);
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
