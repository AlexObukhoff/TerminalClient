/* @file Интерфейс HID устройства. */

#pragma once

#include <SDK/Drivers/IDevice.h>

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
class IHID: virtual public IDevice
{
public: // константы
	/// Событие о новых введённых данных.
	static const char * DataSignal; // = SIGNAL(void data(const QVariantMap &));

public: // методы
	/// Включает/выключает устройство на чтение штрих-кодов. Пикать все равно будет.
	virtual bool enable(bool aEnabled) = 0;

	/// Готов ли к работе (инициализировался успешно, ошибок нет).
	virtual bool isDeviceReady() = 0;

protected:
	virtual ~IHID() {}
};

}} // namespace SDK::Driver

//--------------------------------------------------------------------------------

