/* @file Типы данных ФР ПРИМ. */

#pragma once

// SDK
#include <SDK/Drivers/FR/FiscalDataTypes.h>

// Project
#include "Hardware/FR/FRErrorDescription.h"

//--------------------------------------------------------------------------------
namespace CPrimFR
{
	/// Данные команды и ответа для взаимодействия с протоколом
	typedef QList<QByteArray> TData;

	/// Список данных команд
	typedef QList<TData> TDataList;

	/// Налоги.
	namespace Taxes
	{
		struct SData
		{
			SDK::Driver::TVAT value;    /// значение
			QString description;        /// название
			TData extraData;            /// минимальная сумма, тип, номер тега (для онлайновых ФР)

			SData() : value(0) {}
			SData(int aValue, const QString & aDescription, const TData & aExtraData) : value(aValue), description(aDescription), extraData(aExtraData) {}
		};
	}

	/// Ошибки выполнения команд.
	namespace Errors
	{
		/// Структура описателя ошибок.
		struct SData
		{
			FRError::EType::Enum type;
			QString description;
			bool extraData;

			SData() : type(FRError::EType::Unknown), extraData(false) {}
			SData(FRError::EType::Enum aType, const char * aDescription, bool aExtraData) : type(aType), description(QString::fromUtf8(aDescription)), extraData(aExtraData) {}
		};

		/// Описатель ошибок выполнения команд.
		class SpecificationBase : public CSpecification<char, SData>
		{
		public:
			SpecificationBase()
			{
				setDefault(SData(FRError::EType::Unknown, "Неизвестная", false));
			}

			QString operator[] (char aErrorCode) const
			{
				return CSpecification<char, SData>::operator[](aErrorCode).description;
			}

			virtual QString getDescription(ushort /*aFullErrorCode*/)
			{
				return getDefault().description;
			}

		protected:
			void add(char aCode, FRError::EType::Enum aType, const char * aDescription, bool aExtraData = false)
			{
				append(aCode, SData(aType, aDescription, aExtraData));
			}

			void add(char aCode, const char * aDescription, bool aExtraData = false)
			{
				append(aCode, SData(FRError::EType::Unknown, aDescription, aExtraData));
			}
		};
	}
}

//--------------------------------------------------------------------------------
