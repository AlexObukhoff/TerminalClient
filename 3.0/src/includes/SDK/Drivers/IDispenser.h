/* @file Интерфейс диспенсера. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVector>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IDevice.h>

namespace SDK {
namespace Driver {

typedef QVector<int> TUnitData;

//--------------------------------------------------------------------------------
class IDispenser: public IDevice
{
public:
	/// Предметы выданы.
	static const char * DispensedSignal; // SIGNAL(dispensed(int aUnit, int aItems));

	/// Предметы отбракованы.
	static const char * RejectedSignal; // SIGNAL(rejected(int aUnit, int aItems));

	/// Лоток пуст.
	static const char * UnitEmptySignal; // SIGNAL(unitEmpty(int aUnit));

	/// Количество кассет определено.
	static const char * UnitsDefinedSignal; // SIGNAL(unitsDefined());

public:
	/// Готов ли к работе (инициализировался успешно, ошибок нет).
	virtual bool isDeviceReady(int aUnit = -1) = 0;

	/// Выдать предметы.
	virtual void dispense(int aUnit, int aItems) = 0;

	/// Установить конфигурацию кассет.
	virtual void setUnitList(const TUnitData & aUnitData) = 0;

	/// Получить кол-во лотков диспенсера.
	virtual int units() = 0;

protected:
	virtual ~IDispenser() {}
};

}} // namespace SDK::Driver

Q_DECLARE_METATYPE(SDK::Driver::TUnitData);

//--------------------------------------------------------------------------------
