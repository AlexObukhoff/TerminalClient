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
			FS,             /// ФН.
			SD,             /// Ошибка карты памяти.
			CashAcceptor    /// Ошибка подключаемого купюроприемника.
		};
	}

	struct SData
	{
		QString description;
		EType::Enum type;
		bool extraData;

		SData() : type(EType::FR), extraData(false) {}
		SData(const QString & aDescription, EType::Enum aType = EType::FR, bool aExtraData = false) : description(aDescription), type(aType), extraData(aExtraData) {}
	};

	template <class T>
	class Data : public CSpecification<T, SData>
	{
	public:
		Data()
		{
			setDefault(SData(QString::fromUtf8("Неизвестная"), EType::Unknown));
		}

		void add(T aKey, const char * aDescription, EType::Enum aType = EType::Unknown)
		{
			mBuffer.insert(aKey, SData(QString::fromUtf8(aDescription), aType));
		}

		void add(T aKey, const char * aDescription, bool aExtraData)
		{
			mBuffer.insert(aKey, SData(QString::fromUtf8(aDescription), EType::FR, aExtraData));
		}
	};
}

//--------------------------------------------------------------------------------
