/* @file Типы данных для движка фискальных тегов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
namespace CFR { namespace FiscalFields
{
	// Типы данных.
	namespace ETypes
	{
		enum Enum
		{
			None = 0,
			String,
			Byte,
			ByteArray,
			UnixTime,
			VLN,
			FVLN,
			STLV,
			UINT32,
			UINT16
		};
	};

	// Описатель типов данных.
	namespace Types
	{
		struct SData
		{
			int minSize;
			bool fixSize;
			QString description;

			SData(): minSize(0), fixSize(false) {}
			SData(int aMinSize, const QString & aDescription, bool aFixSize = false): minSize(aMinSize), fixSize(aFixSize), description(aDescription) {}
		};
	}

	//---------------------------------------------------------------------------
	// Обязательность параметра
	namespace ERequired
	{
		enum Enum
		{
			No = 0,
			PM,
			Yes
		};
	}

	//---------------------------------------------------------------------------
	// Структура описателя.
	struct SData
	{
		ETypes::Enum type;
		QString textKey;
		QString translationPF;
		ERequired::Enum required;
		bool isMoney;

		SData(): type(ETypes::None), required(ERequired::No), isMoney(false) {}
		SData(ETypes::Enum aType, const QString & aTextKey, ERequired::Enum aRequired):
			type(aType), textKey(aTextKey), translationPF(""), required(aRequired), isMoney(false) {}
		SData(ETypes::Enum aType, const QString & aTextKey, const QString & aTranslationPF = ""):
			type(aType), textKey(aTextKey), translationPF(aTranslationPF), required(ERequired::No), isMoney(false) {}
		SData(ETypes::Enum aType, const QString & aTextKey, const QString & aTranslationPF, bool aIsMoney):
			type(aType), textKey(aTextKey), translationPF(aTranslationPF), required(ERequired::No), isMoney(aIsMoney) {}
		SData(ETypes::Enum aType, const QString & aTextKey, const QString & aTranslationPF, ERequired::Enum aRequired):
			type(aType), textKey(aTextKey), translationPF(aTranslationPF), required(aRequired), isMoney(false) {}
	};

}}    // namespace CFR::FiscalFields

//---------------------------------------------------------------------------
