/* @file Принтер PrimexNP2511. */

#pragma once

// Modules
#include "Hardware/Printers/SerialPrinterBase.h"

// Project
#include "PrimexPrinterData.h"

//--------------------------------------------------------------------------------
class PrimexNP2511 : public TSerialPrinterBase
{
public:
	PrimexNP2511();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Получить статус;
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Печать чека.
	virtual bool printReceipt(const Tags::TLexemeReceipt & aLexemeReceipt);

	/// Печать штрих-кода.
	virtual bool printBarcode(const QString & aBarcode);

private:
	/// Запросить и сохранить параметры устройства.
	bool processDeviceData(const CPrimexNP2511::TDeviceParametersIt & aIt, QString & aData);

	/// ОБратная промотка.
	bool backFeed(int aCount);
};

//--------------------------------------------------------------------------------
