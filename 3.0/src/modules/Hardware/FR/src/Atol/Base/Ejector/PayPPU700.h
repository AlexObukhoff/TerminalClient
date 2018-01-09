/* @file ФР Pay PPU-700. */

#pragma once

#include "../../Ejector/AtolEjectorFR.h"

/// Константы ФР PayPPU-700.
namespace CPayPPU700
{
	/// Параметры эжектора.
	namespace EjectorParameters
	{
		const char LoopAndPushNotTakenOnTimeout      = '\x40';    /// Петля и вытолкнуть чек по таймауту
		const char NoLoopAndPushNotTakenOnTimeout    = '\xC0';    /// нет петли и вытолкнуть чек по таймауту
		const char NoLoopAndRetractNotTakenOnTimeout = '\x00';    /// нет петли и забрать чек в ретрактор по таймауту
		const char ZReportPush = '\x20';    /// Вытолкнуть Z-отчет.
	}
}

//--------------------------------------------------------------------------------
class PayPPU700 : public AtolEjectorFR<AtolSerialFR>
{
	SET_SUBSERIES("PayPPU700K")

public:
	PayPPU700();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();
};

//--------------------------------------------------------------------------------
