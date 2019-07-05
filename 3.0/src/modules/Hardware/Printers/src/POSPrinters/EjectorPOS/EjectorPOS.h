/* @file POS-принтеры с эжектором. */

#pragma once

#include "Hardware/Printers/PortPOSPrinters.h"

/// Константы POS-принтеров с эжектором.
namespace CEjectorPOS
{
	/// Минимальная длина презентации
	const int MinPresentationLength = 2;
}

//--------------------------------------------------------------------------------
template <class T>
class EjectorPOS : public POSPrinter<T>
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
typedef SerialPOSPrinter<EjectorPOS<TSerialPrinterBase>> TSerialEjectorPOS;
typedef                  EjectorPOS<TLibUSBPrinterBase>  TLibUSBEjectorPOS;

//--------------------------------------------------------------------------------
