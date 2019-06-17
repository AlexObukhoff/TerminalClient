/* @file Прото-сторожевой таймер. */

#pragma once

// SDK
#include <SDK/Drivers/IWatchdog.h>

// Modules
#include "Hardware/Common/ProtoDevice.h"
#include "Hardware/Watchdogs/WatchdogStatusCodes.h"

//--------------------------------------------------------------------------------
class ProtoWatchdog : public ProtoDevice, public MetaDevice<SDK::Driver::IWatchdog>
{
	Q_OBJECT

	SET_DEVICE_TYPE(Watchdog)

signals:
	/// Ключ зарегистрирован.
	void keyRegistered(bool aSuccess);

protected slots:
	/// Обработчик сигнала пинга.
	virtual void onPing() {};

	/// Перезагрузка линии питания.
	virtual bool reset(const QString & /*aLine*/) { return true; };

	/// Зарегистрировать электронный ключ.
	virtual void registerKey() {};
};

//--------------------------------------------------------------------------------
