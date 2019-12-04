/* @file Виртуальный диспенсер. */

#pragma once

#include "Hardware/Common/DeviceBase.h"
#include "Hardware/Common/VirtualDeviceBase.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/Dispensers/DispenserBase.h"

//---------------------------------------------------------------------------------------------
namespace CVirtualDispenser
{
	/// Задержка выдачи 1 предмета.
	const int ItemDispenseDelay = 300;
}

//--------------------------------------------------------------------------------
typedef VirtualDeviceBase<DispenserBase<DeviceBase<ProtoDispenser>>> TVirtualDispenser;

class VirtualDispenser : public TVirtualDispenser
{
public:
	VirtualDispenser();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

protected:
	/// Фильтровать нажатие кнопки(ок).
	virtual void filterKeyEvent(int aKey, const Qt::KeyboardModifiers & aModifiers);

	/// Применить конфигурацию кассет.
	virtual void applyUnitList();

	/// Проверить статус кассеты.
	virtual void checkUnitStatus(TStatusCodes & aStatusCodes, int aUnit);

	/// Выдать.
	virtual void performDispense(int aUnit, int aItems);

	/// Предмет, на котором сработает имитация замятия
	int mJammedItem;

	/// Номер предмета, когда кассета почти пуста.
	int mNearEndCount;
};

//--------------------------------------------------------------------------------
