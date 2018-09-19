/* @file Принтер Citizen PPU-231. */

#pragma once

#include "Hardware/Printers/POSPrinter.h"

//--------------------------------------------------------------------------------
class CitizenPPU231 : public POSPrinter
{
	SET_SUBSERIES("CitizenPPU231")

public:
	CitizenPPU231();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Напечатать чек.
	virtual bool printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt);

	/// Печать штрих-кода.
	virtual bool printBarcode(const QString & aBarcode);
};

//--------------------------------------------------------------------------------
