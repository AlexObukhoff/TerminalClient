/* @file Обобщенные статусы фискальных регистраторов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/DeviceStatus.h>

namespace SDK {
namespace Driver {

//---------------------------------------------------------------------------
namespace EFRStatus
{
	enum Enum
	{
		NoMoneyForSellingBack = 160    /// Не хватает денег для возврата товара.
	};
}

}} // namespace SDK::Driver

Q_DECLARE_METATYPE(SDK::Driver::EFRStatus::Enum);

//--------------------------------------------------------------------------------
