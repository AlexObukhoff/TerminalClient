/* @file ФР семейства Штрих с эжектором c управляемым ретрактором.. */

#pragma once

#include "../Retractor/ShtrihRetractorFRLite.h"

//--------------------------------------------------------------------------------
class ShtrihRetractorFR : public ShtrihRetractorFRLite<ShtrihSerialFR>
{
public:
	ShtrihRetractorFR();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Печать выплаты.
	virtual bool processPayout(double aAmount);

	/// Забрать чек в ретрактор.
	virtual bool retract();
};

//--------------------------------------------------------------------------------
