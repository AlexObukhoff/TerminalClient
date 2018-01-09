/* @file Интерфейс сторожевого устройства. */

#pragma once

// SDK
#include <SDK/Drivers/IDevice.h>

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
class IWatchdog: virtual public IDevice
{
public:
	/// Предметы выданы.
	static const char * KeyRegisteredSignal; // SIGNAL(keyRegistered(bool aSuccess));

public: // методы
	/// Перезагрузка линии питания.
	virtual bool reset(const QString & aLine) = 0;

	/// Зарегистрировать электронный ключ.
	virtual void registerKey() = 0;

protected:
	virtual ~IWatchdog() {}
};

}} // namespace SDK::Driver

//--------------------------------------------------------------------------------
