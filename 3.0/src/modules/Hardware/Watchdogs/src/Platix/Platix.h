/* @file Сторожевой таймер Platix. */

#pragma once

#include "Hardware/Watchdogs/WatchdogBase.h"

//--------------------------------------------------------------------------------
namespace CPlatix
{
	/// Начальный байт пакета.
	const char Sync = '\xC0';

	/// Размер команды.
	const char CommandSize = 4;

	/// Минимальный размер ответного пакета.
	const int MinAnswerSize = 4;

	/// Команды.
	namespace Commands
	{
		const char RebootPC   = '\x00';    /// Ребут компа.
		const char ResetModem = '\x01';    /// Сброс модема.
		const char GetID      = '\x10';    /// Запрос ID.
		const char Poll       = '\x26';    /// Запрос активности компа.
	}
}

//--------------------------------------------------------------------------------
class Platix : public WatchdogBase
{
	SET_SERIES("Platix")

public:
	Platix();

	/// Перезагрузка линии питания.
	virtual bool reset(const QString & aLine);

protected:
	/// Обработчик сигнала пинга.
	virtual void onPing();

	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Подсчитать CRC.
	ushort calcCRC(const QByteArray & aData);

	/// Выполнить команду.
	bool processCommand(char aCommand);

	/// Проверить ответ.
	bool check(const QByteArray & aAnswer);
};

//--------------------------------------------------------------------------------
