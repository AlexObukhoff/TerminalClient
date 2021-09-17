/* @file Сторожевой таймер OSMP 2.5. */

#pragma once

// Modules
#include "Hardware/Watchdogs/WatchdogBase.h"
#include "Hardware/Protocols/Wachdogs/OSMPWD.h"

//--------------------------------------------------------------------------------
class OSMP25: public WatchdogBase
{
	SET_SERIES("OSMP2,5")
	SET_VCOM_DATA(Types::Adapter, ConnectionTypes::VCOMOnly, AdapterTags::FTDI)

public:
	OSMP25();

	/// Перезагрузка линии питания.
	virtual bool reset(const QString & aLine);

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Обработчик сигнала пинга.
	virtual void onPing();

	/// Запуск/останов пинга.
	virtual void setPingEnable(bool aEnabled);

	/// Идентифицирует устройство.
	virtual bool isConnected();

	/// Зарегистрировать электронный ключ.
	virtual void registerKey();

	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Потокол
	OSMPWDProtocol mProtocol;
};

//--------------------------------------------------------------------------------
