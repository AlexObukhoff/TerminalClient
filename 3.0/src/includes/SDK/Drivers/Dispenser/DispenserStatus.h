/* @file Обобщенные статусы устройств выдачи. */

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
/// Обобщенные статусы устройств приема денег. Передаются в пп и служат для внутренних нужд драйвера. Порядок не менять.
namespace EDispenserStatus
{
	enum Enum
	{
		/// OK.
		OK = 1,             /// Хороший статус.
		CassetteOpened = 21 /// Кассета открыта.
	};
}

}} // namespace SDK::Driver

Q_DECLARE_METATYPE(SDK::Driver::EDispenserStatus::Enum);

//--------------------------------------------------------------------------------
