/* @file Описатель фискальных реквизитов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDate>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/FR/FiscalFields.h>

// Modules
#include "FiscalFieldDescriptions.h"

namespace CFR {
namespace FiscalFields {

//---------------------------------------------------------------------------
Data::Data()
{
	TAllData allData = process(0);
	mBuffer = allData.first;
	mDescriptionData = allData.second;

	for (auto it = mBuffer.begin(); it != mBuffer.end(); ++it)
	{
		it->translationPF = QCoreApplication::translate("FiscalFields", it->translationPF.toLatin1());
	}

	foreach(int field, mBuffer.keys())
	{
		checkRFVAT20(field);
	}
}

//---------------------------------------------------------------------------
TAllData Data::process(int aField, const SData & aData)
{
	static TData data;
	static TDescriptionData descriptionData;

	if (!aField)
	{
		return TAllData(data, descriptionData);
	}

	data.insert(aField, aData);
	descriptionData.insert(aField, aData.textKey);

	return TAllData();
}

//---------------------------------------------------------------------------
int Data::getKey(const QString & aTextKey) const
{
	return mDescriptionData.values().contains(aTextKey) ? mDescriptionData.key(aTextKey) : 0;
}

//---------------------------------------------------------------------------
QStringList Data::getTextKeys(const TFields & aFields) const
{
	QStringList result;

	foreach (int field, aFields)
	{
		if (mBuffer.contains(field))
		{
			result << mBuffer[field].textKey;
		}
	}

	return result;
}

//---------------------------------------------------------------------------
QString Data::getTextLog(int aField) const
{
	return QString("fiscal field %1 (%2)").arg(aField).arg(value(aField).textKey.replace("_", " "));
}

//---------------------------------------------------------------------------
QString Data::getLogFromList(const TFields & aFields) const
{
	QStringList data;

	foreach(int field, aFields)
	{
		data << getTextLog(field);
	}

	return data.join(", ").replace("fiscal field ", "");
}

//---------------------------------------------------------------------------
void Data::checkRFVAT20(int aField)
{
	if (isRFVAT20() && TaxAmountFields().contains(aField))
	{
		mBuffer[aField].translationPF.replace("18", "20");
		mDescriptionData[aField].replace("18", "20");
	}
}

}}    // namespace CFR::FiscalFields

//---------------------------------------------------------------------------
