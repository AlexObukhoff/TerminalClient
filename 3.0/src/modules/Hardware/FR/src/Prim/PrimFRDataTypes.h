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

	/// Ошибки.
	namespace Errors
	{
		class ExtraDataBase
		{
		public:
			virtual QString value(char /*aErrorCode*/, char aErrorReason)
			{
				return QString::number(uchar(aErrorReason));
			}
		};
	}
}

//--------------------------------------------------------------------------------
