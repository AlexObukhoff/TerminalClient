/* @file Виртуальный купюроприемник. */

#pragma once

#include "Hardware/Common/VirtualDeviceBase.h"
#include "Hardware/CashAcceptors/CashAcceptorBase.h"

//---------------------------------------------------------------------------------------------
typedef VirtualDeviceBase<CashAcceptorBase<DeviceBase<ProtoCashAcceptor>>> TVirtualCashAcceptor;

class VirtualCashAcceptor : public TVirtualCashAcceptor
{
public:
	VirtualCashAcceptor();

#pragma region IDevice interface
	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);
#pragma endregion

	/// Нужен для имитации зачисления произвольной купюры в тестах расчета комиссий
	virtual void testStack(double aAmount);

#pragma region ICashAcceptor interface
	/// Активировать/деактивировать приём.
	virtual bool setEnable(bool aEnabled);

	/// Принять купюру.
	virtual bool stack();

	/// Вернуть купюру.
	virtual bool reject();
#pragma endregion

private:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Фильтровать нажатие кнопки(ок).
	virtual void filterKeyEvent(int aKey, const Qt::KeyboardModifiers & aModifiers);

	/// Загрузка таблицы номиналов.
	virtual bool loadParTable();

	/// Покинуть Escrow-состояние.
	bool leaveEscrow(int aStatusCode);

	/// Cтатус-коды со срабатыванием при стекировании.
	TStatusCodes mStackedStatusCodes;

	/// Количество купюр, принимаемых за 1 заход.
	int mNotesPerEscrow;
};

//-----------------------------------------------------------------------------------------------------------
