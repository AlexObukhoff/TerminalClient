/* @file Принтеры Swecoin. */

#pragma once

#include "Hardware/Printers/PortPrintersBase.h"

//--------------------------------------------------------------------------------
class SwecoinPrinter : public TSerialPrinterBase
{
public:
	SwecoinPrinter();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();
};

//--------------------------------------------------------------------------------
