/* @file ФР семейства Штрих с ограниченным управлением эжектором. */

#pragma once

#include "../Online/ShtrihOnlineFRBase.h"
#include "../Base/ShtrihFRBase.h"
#include "../Base/ShtrihSerialFR.h"

//--------------------------------------------------------------------------------
template <class T>
class ShtrihRetractorFRLite : public T
{
	SET_SUBSERIES("Ejector")

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Отрезка.
	virtual bool cut();

	/// Добавить общие статусы.
	virtual void appendStatusCodes(ushort aFlags, TStatusCodes & aStatusCodes);

	/// Получить таймаут незабранного чека.
	virtual uchar getLeftReceiptTimeout();

	/// Выполнить действия после установки параметров системной таблицы.
	void postSettingAction();
};

//--------------------------------------------------------------------------------
