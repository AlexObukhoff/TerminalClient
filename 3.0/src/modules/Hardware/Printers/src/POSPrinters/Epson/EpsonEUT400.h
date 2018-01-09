/* @file Принтеры семейства Epson EU-T400. */

#pragma once

#include "Hardware/Printers/POSPrinter.h"

//--------------------------------------------------------------------------------
class EpsonEUT400 : public POSPrinter
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

	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData();

	/// Установить memory switch.
	bool setMemorySwitch(char aNumber, char aValue);

	/// Получить memory-switch.
	bool getMemorySwitch(char aNumber, char & aValue);

	/// Выполнить сброс.
	bool reset();
};

//--------------------------------------------------------------------------------
