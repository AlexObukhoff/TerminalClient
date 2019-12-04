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
	// Обязательность тега.
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
	// Обобщенный тип (класс) тега.
	namespace EClassType
	{
		enum Enum
		{
			Default = 0,
			Money,
			INN
		};
	}

	//---------------------------------------------------------------------------
	// Структура описателя тега.
	struct SData
	{
		ETypes::Enum type;             /// Тип.
		QString textKey;               /// Текстовый ключ.
		QString translationPF;         /// Перевод для печатной формы (ПФ).
		ERequired::Enum required;      /// Обязательность тега.
		EClassType::Enum classType;    /// Обобщенный тип (класс) тега.

		SData(): type(ETypes::None), required(ERequired::No), classType(EClassType::Default) {}

		SData(ETypes::Enum aType, const QString & aTextKey, ERequired::Enum aRequired):
			type(aType), textKey(aTextKey), translationPF(""), required(aRequired), classType(EClassType::Default) {}

		SData(ETypes::Enum aType, const QString & aTextKey, EClassType::Enum aClassType):
			type(aType), textKey(aTextKey), translationPF(""), required(ERequired::No), classType(aClassType) {}

		SData(ETypes::Enum aType, const QString & aTextKey, const QString & aTranslationPF = ""):
			type(aType), textKey(aTextKey), translationPF(aTranslationPF), required(ERequired::No), classType(EClassType::Default) {}

		SData(ETypes::Enum aType, const QString & aTextKey, const QString & aTranslationPF, EClassType::Enum aClassType):
			type(aType), textKey(aTextKey), translationPF(aTranslationPF), required(ERequired::No), classType(aClassType) {}

		SData(ETypes::Enum aType, const QString & aTextKey, const QString & aTranslationPF, ERequired::Enum aRequired, EClassType::Enum aClassType = EClassType::Default):
			type(aType), textKey(aTextKey), translationPF(aTranslationPF), required(aRequired), classType(aClassType) {}

		bool isString() const { return type == ETypes::String;   }
		bool isSTLV()   const { return type == ETypes::STLV;     }
		bool isTime()   const { return type == ETypes::UnixTime; }

		bool isMoney()  const { return classType == EClassType::Money; }
		bool isINN()    const { return classType == EClassType::INN;   }
	};

}} // namespace CFR::FiscalFields

//---------------------------------------------------------------------------
