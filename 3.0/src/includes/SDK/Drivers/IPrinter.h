/* @file Интерфейс принтера. */

#pragma once

// SDK
#include <SDK/Drivers/IDevice.h>

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
class IPrinter: public IDevice
{
public: // методы
	/// Напечатать.
	virtual bool print(const QStringList & aStrings) = 0;

	/// Готов ли к печати.
	virtual bool isDeviceReady(bool aOnline) = 0;

protected:
	virtual ~IPrinter() {}
};

}} // namespace SDK::Driver

//--------------------------------------------------------------------------------
