/* @file Принтеры семейства Epson EU-T400. */

#pragma once

#include "EpsonPrinters.h"

//--------------------------------------------------------------------------------
class EpsonEUT400 : public EpsonSerialPrinter
{
	SET_SUBSERIES("EpsonEUT400")

public:
	EpsonEUT400();

	/// Инициализация устройства.
	virtual bool updateParametersOut();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();
};

//--------------------------------------------------------------------------------
