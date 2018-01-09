/* @file Данные сторожевого таймера LDog. */

#pragma once

#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CLDog
{
	/// ACK.
	const char ACK = '\x50';

	/// NAK.
	const char NAK = '\x80';

	/// Таймауты.
	namespace Timeouts
	{
		/// Стартовый таймаут сброса ПК, [с].
		const ushort Start = 5 * 60;

		/// Периодический таймаут сброса ПК, [с].
		const ushort PCReset = 2 * 60;

		/// Время на замыкание контактов, [мс].
		const ushort PowerOff = 1900;
	}

	/// Интервалы после подачи команд, [мс].
	namespace Intervals
	{
		/// Ребут ПК/модема.
		const int PowerOff = Timeouts::PowerOff + 500;

		/// Запись данных.
		const int Write = 2000;

		/// Чтение данных.
		const int Read = 1000;
	}

	//--------------------------------------------------------------------------------
	/// Команды.
	namespace Commands
	{
		const char RebootPC         = 0x00;    /// Ребут ПК.
		const char ResetModem       = 0x01;    /// Сброс модема.
		const char PCEnable         = 0x03;    /// Запрос активности компа (сигнал об отсутствии зависания).
		const char GetTimeouts      = 0x04;    /// Запрос таймаутов.
		const char SetTimeouts      = 0x05;    /// Установка таймаутов.
		const char ResetChassisData = 0x06;    /// Сброс флагов вскрытия корпуса.
		const char GetChassisData   = 0x07;    /// Запрос флагов вскрытия корпуса.
		const char GetRebootPCTime  = 0x09;    /// Запрос времени до сброса ПК.
		const char GetSensorsData   = 0x10;    /// Запрос счетчиков вскрытия корпуса.
		const char GetDeviceID      = 0x11;    /// Запрос ИД платы.
		const char SetDeviceID      = 0x12;    /// Установка ИД платы.

		class CData : public CSpecification<char, bool>
		{
		public:
			CData()
			{
				append(GetTimeouts,      true);
				append(GetChassisData,   true);
				append(GetRebootPCTime,  true);
				append(GetSensorsData,   true);
				append(GetDeviceID,      true);

				setDefault(false);
			}
		};

		static CData Data;
	}

	/// Ключ для выполнения сброса флагов вскрытия корпуса.
	const char ResetKey[] = "\x34\x12\x30\xF4\x0A\xFE\x05\x23\xDE\xAF\x12\xFE\x63\x1E\x1F\x2F\x2F\x1D\x8A\x6E\xFF\x25\x4F\x16\x2E\x4E\x1F\xF2\xAF\x12";
};

//--------------------------------------------------------------------------------
