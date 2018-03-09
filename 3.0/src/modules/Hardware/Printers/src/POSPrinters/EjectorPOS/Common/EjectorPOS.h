/* @file POS-принтеры с эжектором. */

#pragma once

/// Константы POS-принтеров с эжектором.
namespace CEjectorPOS
{
	/// Минимальная длина презентации
	const int MinPresentationLength = 2;
}

#include "Hardware/Printers/POSPrinter.h"
//--------------------------------------------------------------------------------
class EjectorPOS : public POSPrinter
{
public:
	EjectorPOS();

	/// Устанавливает конфигурацию устройству.
	virtual void setDeviceConfiguration(const QVariantMap & aConfiguration);

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();
};

//--------------------------------------------------------------------------------
