/* @file Принтер Gebe. */

#pragma once

#include "Hardware/Printers/SerialPrinterBase.h"

//--------------------------------------------------------------------------------
class GeBe : public TSerialPrinterBase
{
public:
	GeBe();

protected:
	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();
};

//--------------------------------------------------------------------------------
