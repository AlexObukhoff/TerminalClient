/* @file Сторожевой таймер OSMP. */

#pragma once

// Modules
#include "Hardware/Watchdogs/WatchdogBase.h"

// Project
#include "OSMPWdData.h"

//--------------------------------------------------------------------------------
namespace COSMP
{
	/// Проверка на неправильную идентификацию
	const QByteArray WrongDeviceCheck = QByteArray::fromRawData("\xa5\x00\x00\x5b", 4);

	/// Пауза при переоткрытии порта, [мс]
	const int ReopenPortPause = 1000;
}

//--------------------------------------------------------------------------------
class OSMP: public WatchdogBase
{
	typedef QMap<EOSMPCommandId::Enum, const char *> TData;

	SET_SERIES("OSMP")

public:
	OSMP();

	/// Перезагрузка линии питания.
	virtual bool reset(const QString & aLine);

protected:
	/// Обработчик сигнала пинга.
	virtual void onPing();

	/// Запуск/останов пинга.
	virtual void setPingEnable(bool aEnabled);

	/// Идентифицирует устройство.
	virtual bool isConnected();

	/// Выполнить команду.
	bool performCommand(const QByteArray & aCommand, QByteArray * aAnswer = nullptr);

	/// Данные протокола сторожевого таймера.
	TData mData;
};

//--------------------------------------------------------------------------------
