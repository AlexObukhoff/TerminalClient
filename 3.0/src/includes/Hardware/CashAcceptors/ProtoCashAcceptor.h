/* @file Прото-устройство приема денег. */

#pragma once

// SDK
#include <SDK/Drivers/ICashAcceptor.h>

// Modules
#include "Hardware/Common/ProtoDevice.h"

//--------------------------------------------------------------------------------
class ProtoCashAcceptor : public ProtoDevice, public SDK::Driver::ICashAcceptor
{
	Q_OBJECT

	SET_DEVICE_TYPE(BillAcceptor)

signals:
	/// Купюра готова к укладке.
	void escrow(SDK::Driver::SPar aNote);

	/// Купюра уложена.
	void stacked(SDK::Driver::TParList aNotes);

protected slots:
	/// Извещает верхнюю логику о завершении отключения устройства приема денег.
	virtual void onSendDisabled() {}

	/// Устанавливает запрещения списка номиналов.
	virtual void employParList() {}

	/// Восстановление статусов для отправки наверх после отключения поллинга.
	virtual void restoreStatuses() {}

	/// Выполняет физические действия по включению/выключению устрйоства.
	virtual void processEnable(bool /*aEnabled*/) {}

	/// Нужен для имитации зачисления произвольной купюры в тестах расчета комиссий
	virtual void testStack(double /*aAmount*/) {}
};

//--------------------------------------------------------------------------------
