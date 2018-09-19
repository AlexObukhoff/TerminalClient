/* @file Базовый ФР Pay на протоколе Штрих. */

#pragma once

#include "../../Retractor/ShtrihRetractorFRLite.h"
#include "Hardware/FR/PayPrinters.h"

//--------------------------------------------------------------------------------
template<class T>
class PayFRBase : public ShtrihRetractorFRLite<T>
{
public:
	PayFRBase();

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Запросить и вывести в лог критичные параметры ФР.
	virtual void processDeviceData();

	/// Снять Z-отчет.
	virtual bool execZReport(bool aAuto);

	/// Добавить общие статусы.
	virtual void appendStatusCodes(ushort aFlags, TStatusCodes & aStatusCodes);

	/// Забрать чек в ретрактор.
	virtual bool retract();

	/// Id модели подключенного принтера.
	uchar mPrinterModelId;
};

//--------------------------------------------------------------------------------
