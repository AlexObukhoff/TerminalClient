/* @file Базовый класс AT-совместимого модема. */

#pragma once

// Modules
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Common/SerialDeviceBase.h"

// Project
#include "ATGSMModemConstants.h"
//--------------------------------------------------------------------------------
class ATModemBase : public SerialDeviceBase<PortDeviceBase<DeviceBase<ProtoModem>>>
{
public:
	ATModemBase();

	/// Сброс.
	virtual bool reset();

	/// Устанавливает строку инициализации.
	virtual bool setInitString(const QString & aInitString);

protected:
	/// Идентифицирует устройство.
	virtual bool isConnected();

	/// Включение/выключение локального эха.
	bool enableLocalEcho(bool aEnable);

	/// Составляет командую строку, добавляя необходимые префиксы и окончания.
	QByteArray makeCommand(const QString & aCommand);

	/// Посылает команду и читает ответ на неё.
	bool processCommand(const QByteArray & aCommand, int aTimeout = CATGSMModem::Timeouts::Default);
	bool processCommand(const QByteArray & aCommand, QByteArray & aAnswer, int aTimeout = CATGSMModem::Timeouts::Default);

	/// Разбор имени модема
	virtual void setDeviceName(const QByteArray & aFullName);

	/// Теребим модем командой AT
	bool checkAT(int aTimeout);

	/// Таймаут запроса конфигурации модема
	int mModemConfigTimeout;
};

//--------------------------------------------------------------------------------
