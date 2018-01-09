/* @file Данные протокола сторожевого таймера Alarm. */

#pragma once

#include "Hardware/Watchdogs/WatchdogStatusCodes.h"

//----------------------------------------------------------------------------
namespace CAlarm
{
	/// Модель.
	char Model[] = "Alarm";

	/// Таймаут по умолчанию, [мс].
	const int DefaultTimeout = 1000;

	namespace Commands
	{
		const char Ping = '\x88';    // Сброс таймера. Необходимо проводить не реже 1 раза в 63 сек. Остановить нельзя, поэтому не используется.
		const char ResetModem = '\x84';    // Ребут модема.
		const char GetSwitchState  = '\x80';
		const char GetOutputsState = '\xC0';
	}

	typedef QPair<uchar, uchar> TInterval;

	/// Спецификация состояний сенсоров.
	class CCommandIntervals : public CSpecification<char, TInterval>
	{
	public:
		CCommandIntervals()
		{
			append('\xC0', TInterval(0xC0, 0xC7));
			append('\x80', TInterval(0x00, 0x1F));
		}
	};

	//--------------------------------------------------------------------------------
	/// Спецификация состояний сенсоров.
	class CSensorCodeSpecification : public CSpecification<int, int>
	{
	public:
		CSensorCodeSpecification()
		{
			append(0, WatchdogStatusCode::Warning::Safe);
			append(1, WatchdogStatusCode::Warning::UpperUnit);
			append(2, WatchdogStatusCode::Warning::LowerUnit);
			append(5, WatchdogStatusCode::Warning::Tilt);
			append(6, WatchdogStatusCode::Warning::Kick);
		}
	};

	static CSensorCodeSpecification SensorCodeSpecification;
}

//--------------------------------------------------------------------------------
