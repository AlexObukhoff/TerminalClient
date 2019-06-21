/* @file Данные протокола сторожевого таймера OSMP 2.5. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

//----------------------------------------------------------------------------
namespace COSMP25
{
	/// Тег модели для идентификации.
	const char ModelTag[] = "2.5.";

	/// Интервал пинга, [мс].
	const int PingInterval = 10000;

	/// Интервал пинга, [с].
	const uchar PingTimeout = 30;

	/// Пауза при отключении питания модема, [с].
	const uchar ModemResettingPause = 2;

	/// Пауза при отключении питания PC, [с].
	const uchar PCResettingPause = 3;

	/// Истек таймаут при регистрации ключа.
	const char KeyRegisteringExpired = '\x05';

	/// Максимальное количество ключей.
	const int MaxKeys = 32;

	/// Интервал включения питания PC, [с].
	const int PCWakingUpInterval = 30 * 60;

	/// Формат представления даты и времени для вывода в лог.
	const char TimeLogFormat[] = "hh:mm:ss";

	/// Временной лаг установки времени перезагрузки PC, [с].
	const int PCWakingUpLag = 5;

	/// Команды.
	namespace Commands
	{
		const char GetVersion       = '\x00';        /// Версия
		const char SerialNumber     = '\x01';        /// Серийный номер
		const char ResetModem       = '\x02';        /// Сброс модема
		const char SetModemPause[]  = "\x02\x02";    /// Установка времени сброса модема
		const char SetPingEnable    = '\x03';        /// Включение таймера сторожа 
		const char SetPingTimeout[] = "\x03\x02";    /// Установка времени сторожа таймера
		const char SetPingDisable   = '\x04';        /// Отключение таймера сторожа
		const char Ping             = '\x05';        /// СБрос времени сторожа
		const char ResetClock       = '\x06';        /// Сброс часов
		const char ResetPC          = '\x0C';        /// Сброс PC
		const char SetPCPause[]     = "\x0C\x02";    /// Установка таймаута сброса PC
		const char WriteKey         = '\x11';        /// Запись ключа
		const char ReadKey          = '\x12';        /// Чтение ключа
		const char PCWakeUpTime     = '\x15';        /// Установка/получение времени включения PC

		const QByteArray ResetPCWakeUpTime = QByteArray::fromRawData("\x15\x00", 2);    /// Сброс времени включения PC
	}
}

//--------------------------------------------------------------------------------
