/* @file Принтеры семейства Epson. */

#pragma once

#include "Hardware/Printers/PortPOSPrinters.h"
#include "EpsonConstants.h"

//--------------------------------------------------------------------------------
template <class T>
class EpsonPrinter : public T
{
	SET_SUBSERIES("Epson")

public:
	EpsonPrinter();

protected:
	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData();

	/// Получить параметры устройства.
	bool getDeviceData(const QByteArray & aCommand, QByteArray & aAnswer);

	/// Установить memory switch.
	bool setMemorySwitch(char aNumber, char aValue);

	/// Получить memory-switch.
	bool getMemorySwitch(char aNumber, char & aValue);

	/// Выполнить сброс.
	bool reset();

	/// Таймаут получения данных устройства.
	int mDeviceDataTimeout;
};

//--------------------------------------------------------------------------------
typedef EpsonPrinter<TSerialPOSPrinter> EpsonSerialPrinter;
typedef EpsonPrinter<TTCPPOSPrinter> EpsonTCPPrinter;

//--------------------------------------------------------------------------------
