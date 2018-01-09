/* @file Коды состояний сторожевых таймеров. */

#pragma once

#include "Hardware/Common/BaseStatus.h"

//--------------------------------------------------------------------------------
namespace WatchdogStatusCode
{
	/// Предупреждения.
	namespace Warning
	{
		const int Door          = 410;    /// Дверь.
		const int Safe          = 411;    /// Сейф.
		const int UpperUnit     = 412;    /// Верхний модуль.
		const int LowerUnit     = 413;    /// Верхний модуль.
		const int Kick          = 414;    /// Удар.
		const int Tilt          = 415;    /// Наклон.
		const int Power         = 416;    /// Питание.
		const int UPSLowBattery = 417;    /// Батарея ИБП разряжена.
	}

	/// Ошибки.
	namespace Error
	{
		const int SensorBlock     = 430;    /// Блок датчиков.
		const int Temperature     = 431;    /// Датчик температуры.
		const int PCVoltageBlock  = 432;    /// Блок контроля напряжения PC.
		const int PCVoltage       = 433;    /// Нет напряжения 12 В PC.
		const int UPSVoltageBlock = 434;    /// Блок контроля напряжения UPS.
		const int UPSVoltage      = 435;    /// Нет напряжения 220 В на входе ИБП.
	}
}

//--------------------------------------------------------------------------------
