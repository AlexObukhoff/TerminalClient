/* @file Диспенсер. */

//SDK
#include <SDK/Drivers/Dispenser/DispenserStatus.h>

#pragma once

//--------------------------------------------------------------------------------
template <class T>
class DispenserBase : public T
{
public:
	DispenserBase();

	/// Готов ли к работе (инициализировался успешно, ошибок нет).
	virtual bool isDeviceReady(int aUnit = -1);

	/// Установить конфигурацию кассет.
	virtual void setCashList(const SDK::Driver::TUnitData & aUnitData);

	/// Получить кол-во кассет.
	virtual int units();

protected:
	/// Применить конфигурацию кассет.
	virtual void applyCashList();

	/// Выдать.
	virtual void dispense(int aUnit, int aItems);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Анализирует коды статусов устройства и фильтрует несуществующие статусы для нижней логики.
	virtual void cleanStatusCodes(TStatusCodes & aStatusCodes);

	/// Отправить статус-коды.
	virtual void emitStatusCodes(TStatusCollection & aStatusCollection, int aExtendedStatus = SDK::Driver::EStatus::Actual);

	/// Проверить статус кассеты.
	virtual void checkUnitStatus(TStatusCodes & aStatusCodes, int aUnit);

	/// Фоновая логика при появлении определенных состояний устройства.
	virtual void postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection);

	/// Упорядочить конфигурацию кассет в соответствии с новым их количеством.
	void adjustCashList(bool aConfigData);

	/// Кол-во кассет.
	int mUnits;

	/// Данные о количестве предметов в кассетах из конфигурации.
	SDK::Driver::TUnitData mUnitConfigData;

	/// Данные о количестве предметов в кассетах.
	SDK::Driver::TUnitData mUnitData;

	/// Необходимо сообщить количество кассет.
	bool mNeedGetUnits;

	/// Ошибка установки содержимого кассет.
	bool mCashError;
};

//--------------------------------------------------------------------------------
