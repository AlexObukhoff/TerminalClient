/* @file Данные моделей устройств на протоколе V2e. */

#pragma once

#include "V2eModelData.h"

//--------------------------------------------------------------------------------
CV2e::ModelData::ModelData()
{
	add(CV2e::Models::Aurora, TModelKeys() << "VA" << "VE" << "VH" << "VV" << "LA" << "SA", true);
	add(CV2e::Models::Falcon, TModelKeys() << "W", true);
	add(CV2e::Models::Argus,  TModelKeys() << "A" << "B" << "C" << "D" << "E" << "F" << "G" << "H" << "J" << "K" << "P" << "S" << "Y" << "1" << "2");
}

//--------------------------------------------------------------------------------
SBaseModelData CV2e::ModelData::getData(const QString & aKey)
{
	for (auto it = data().begin(); it != data().end(); ++it)
	{
		foreach (const QString & key, it.key())
		{
			if (aKey.startsWith(key))
			{
				return it.value();
			}
		}
	}

	return SBaseModelData("", false);
}

//--------------------------------------------------------------------------------
void CV2e::ModelData::add(const QString & aName, const CV2e::TModelKeys & aModelKeys, bool aVerified)
{
	append(aModelKeys, SBaseModelData(aName, aVerified));
}

//--------------------------------------------------------------------------------
bool operator<(const CV2e::TModelKeys & aKeys1, const CV2e::TModelKeys & aKeys2)
{
	QStringList key1 = aKeys1.toList();
	qSort(key1);

	QStringList key2 = aKeys2.toList();
	qSort(key2);

	return key1.join("") < key2.join("");
}

//--------------------------------------------------------------------------------
