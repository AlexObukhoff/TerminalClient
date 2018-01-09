/* @file Приоритет, с которым производится автопоиск устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace Driver {

//---------------------------------------------------------------------------
namespace EDetectingPriority
{
	enum Enum
	{
		Fallback,
		Low,
		Normal,
		High,
		VeryHigh
	};
}

}} // namespace SDK::Driver

Q_DECLARE_METATYPE(SDK::Driver::EDetectingPriority::Enum);

//------------------------------------------------------------------------------
