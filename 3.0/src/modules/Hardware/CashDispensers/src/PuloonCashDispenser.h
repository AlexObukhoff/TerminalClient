/* @file Диспенсер купюр Puloon. */

#pragma once

// Modules
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/Common/DeviceCodeSpecification.h"
#include "../../Protocols/CashAcceptor/Puloon/src/Puloon.h"

// Project
#include "Hardware/Dispensers/DispenserBase.h"

//--------------------------------------------------------------------------------
class PuloonLCDM : public DispenserBase<SerialDeviceBase<PortPollingDeviceBase<ProtoDispenser>>>
{
	SET_SERIES("Puloon")

public:
	PuloonLCDM();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Получение статуса.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Выдать.
	virtual void performDispense(int aUnit, int aItems);

	/// Выполнить команду.
	TResult processCommand(char aCommand, QByteArray * aAnswer = nullptr);
	TResult processCommand(char aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr, int aTimeout = 0);

	/// Вывод в лог изменения статуса.
	void logStatusChanging(const SDeviceCodeSpecification & aStatusSpecification, const TDeviceCodeSpecifications & aSensorSpecifications);

	/// Протокол.
	Puloon mProtocol;

	/// Последние девайс-коды для логгирования смены статуса.
	QByteArray mLastDeviceStatusCodes;
};

//--------------------------------------------------------------------------------
