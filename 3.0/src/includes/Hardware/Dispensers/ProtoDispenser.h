/* @file Прото-диспенсер. */

#pragma once

// SDK
#include <SDK/Drivers/IDispenser.h>

// Modules
#include "Hardware/Common/ProtoDevice.h"

//--------------------------------------------------------------------------------
class ProtoDispenser : public ProtoDevice, public SDK::Driver::IDispenser
{
	Q_OBJECT

	SET_DEVICE_TYPE(Dispenser)

signals:
	/// Были отданы предметы.
	void dispensed(int aUnit, int aItems);

	/// Сигнал отбора денег из щели во внутренний ящик (предположительно).
	void rejected(int aUnit, int aItems);

	/// Лоток пуст.
	void unitEmpty(int aUnit);

	/// Количество кассет определено.
	void unitsDefined();

protected slots:
	/// Применить конфигурацию кассет.
	virtual void applyCashList() {};

	/// Выдать.
	virtual void dispense(int /*aUnit*/, int /*aItems*/) {};

	/// Выдать.
	virtual void performDispense(int /*aUnit*/, int /*aItems*/) {};
};

//--------------------------------------------------------------------------------
