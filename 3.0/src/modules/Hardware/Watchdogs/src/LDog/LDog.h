/* @file Сторожевой таймер LDog. */

#pragma once

// Modules
#include "Hardware/Protocols/Wachdogs/LDogWD.h"

// Project
#include "Hardware/Watchdogs/WatchdogBase.h"

//--------------------------------------------------------------------------------
class LDog: public WatchdogBase
{
	SET_SERIES("LDog")

public:
	LDog();

	/// Перезагрузка линии питания.
	virtual bool reset(const QString & aLine);

protected:
	/// Идентифицирует устройство.
	virtual bool isConnected();

	/// Обработчик сигнала пинга.
	virtual void onPing();

	/// Запуск/останов пинга.
	virtual void setPingEnable(bool aEnabled);

	/// Выполнить команду.
	TResult processCommand(char aCommand, QByteArray * aAnswer = nullptr);
	TResult processCommand(char aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Установить таймауты.
	bool setToumeouts(bool aEnabled);

	/// Выдержать паузу перед запросом в устройство.
	void makePause(char aCommand);

	/// Допустимое время отправки команды.
	QDateTime mNextRequestTime;

	/// Протокол.
	LDogWDProtocol mProtocol;
};

//--------------------------------------------------------------------------------
