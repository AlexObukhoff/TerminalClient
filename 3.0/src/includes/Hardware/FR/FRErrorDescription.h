/* @file Описатель ошибок ФР. */

#pragma once

#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace FRError
{
	/// Типы ошибок в ответе на команды ФР.
	namespace EType
	{
		enum Enum
		{
			Unknown = 0,    /// Нет ошибок.
			Retry,          /// повторить команду еще раз.
			FR,             /// ФР.
			Printer,        /// Принтер.
			FM,             /// Фискальная память.
			EKLZ,           /// ЭКЛЗ.
			FS              /// ФН.
		};
	}

	struct SData
	{
		QString description;
		EType::Enum type;

		SData() : type(EType::FR) {}
		SData(const QString & aDescription, EType::Enum aType = EType::FR) : description(aDescription), type(aType) {}
	};

	class CData : public CSpecification<char, SData>
	{
	public:
		CData()
		{
			setDefault(SData(QString::fromUtf8("Неизвестная"), EType::Unknown));
		}

		void add(char aKey, const char * aDescription, EType::Enum aType = EType::Unknown)
		{
			mBuffer.insert(aKey, SData(QString::fromUtf8(aDescription), aType));
		}
	};
}

//--------------------------------------------------------------------------------
