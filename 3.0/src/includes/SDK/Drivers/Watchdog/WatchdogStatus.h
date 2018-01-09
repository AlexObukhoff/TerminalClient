/* @file Обобщенные статусы сторожевых таймеров. */

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
namespace EWatchdogStatus
{
	enum Enum
	{
		EnterServiceMenu = 140,    /// Войти в сервисное меню.
		LockTerminal               /// Заблокировать терминал.
	};
}

}} // namespace SDK::Driver

Q_DECLARE_METATYPE(SDK::Driver::EWatchdogStatus::Enum);

//--------------------------------------------------------------------------------
