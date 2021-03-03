/* @file Приоритет значимости параметров плагинов. Имеет значение при разрешении конфликтов при создании плагина,
если для одного имени устройства есть группа плагинов с пересекающимися свойствами. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace Plugin {

//---------------------------------------------------------------------------
namespace EImportanceLevel
{
	enum Enum
	{
		Low,
		Normal,
		High
	};
}

}} // namespace SDK::Driver

Q_DECLARE_METATYPE(SDK::Plugin::EImportanceLevel::Enum);

//------------------------------------------------------------------------------
