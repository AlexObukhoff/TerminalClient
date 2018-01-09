/* @file Уровени тревожности статус-кодов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
/// Общие состояния устройств - уровень тревожности.
namespace EWarningLevel
{
	enum Enum
	{
		OK,      /// Нет ошибок.
		Warning, /// Предупреждение.
		Error    /// Ошибка.
	};
}

}} // namespace SDK::Driver

//--------------------------------------------------------------------------------
Q_DECLARE_METATYPE(SDK::Driver::EWarningLevel::Enum);

//--------------------------------------------------------------------------------

