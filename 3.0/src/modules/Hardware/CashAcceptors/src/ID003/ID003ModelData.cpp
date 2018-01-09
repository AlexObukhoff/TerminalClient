/* @file Данные моделей устройств на протоколе ID003. */

#pragma once

#include "ID003ModelData.h"

using namespace CID003::ProtocolData;

//--------------------------------------------------------------------------------
CIdentification::CIdentification()
{
	append(Alias::ID003,    QString("^%1\\(%1\\)\\-?%1\\-?%1? *(()ID003\\-%1) *[vV](%1\\-%1) *%1 *%1 *$").arg(IdLexeme));
	append(Alias::ID003Ext, QString("^%1\\(%1\\)\\-?%1\\-?%1? *(((ID003)))%1(((\\d{2}\\w{3}\\d{2}))) *%1 *$").arg(IdLexeme));
	append(Alias::BDP,      QString("^%1\\(%1\\)\\-(%2){2}\\-?%1 *(%1?\\-?BDP%1)[vV](%1\\-?%1) *%1 *%1 *$").arg(IdLexeme).arg(Lexeme));
	append(Alias::OP003,    QString("^(%1) *%1 *%1 *((OP003\\-%1)) *[vV](%1\\-?%1?) *%1? *%1? *$").arg(IdLexeme));
}

//--------------------------------------------------------------------------------
CID003::ModelData::ModelData()
{
	QString type = "model_number";
	SModelData ardaks("MoneyControls Unknown", type);
	ardaks.models.add("AE", "MoneyControls Ardac Elite", true);
	ardaks.models.add("A5", "MoneyControls Ardac 5");
	append('A', SModelData(ardaks));

	add('D', "JCM DBV", true);
	add('E', "JCM EBA");
	add('F', "Cashcode MFL", true);
	add('G', "GPT Falcon");
	add('P', "JCM Taiko PUB-7", true);
	add('R', "KBSP BNR100");

	type = "stacker_type";
	SModelData cashcodes("Cashcode Unknown", type);
	cashcodes.models.add("SW", "Cashcode SM", true);
	cashcodes.models.add("MW", "Cashcode MSM", true);
	cashcodes.models.add("FL", "Cashcode MFL", true);
	cashcodes.models.add("VU", "Cashcode MVU");
	append('S', SModelData(cashcodes));

	add('i', "JCM IVISION", true);
	add('U', "JCM UBA", true);
	add('V', "Cashcode MVU");
	add('W', "JCM WBA", true);
	add('T', "JCM TBV-100-FSH", true);
}

//--------------------------------------------------------------------------------
SBaseModelData CID003::ModelData::getData(char aCode, const QString & aNumber, const QString & aStackerType)
{
	SModelData modelData = data()[aCode];

	if (modelData.type == "model_number")
	{
		return modelData.models[aNumber];
	}

	if (modelData.type == "stacker_type")
	{
		return modelData.models[aStackerType];
	}

	return modelData;
}

//--------------------------------------------------------------------------------
void CID003::ModelData::add(char aId, const QString & aName, bool aVerified)
{
	append(aId, SModelData(aName, aVerified));
}

//--------------------------------------------------------------------------------
CID003::SModelData::SModelData(const QString & aDefaultName, const QString & aType) : type(aType)
{
	name = aDefaultName;
	models.setDefault(SBaseModelData(aDefaultName, false));
}

//--------------------------------------------------------------------------------
