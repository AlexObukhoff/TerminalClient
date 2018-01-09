/* @file Принтеры семейства Star c эжектором. */

#pragma once

#include "StarPrinters.h"

//--------------------------------------------------------------------------------
class EjectorStarPrinter : public StarPrinter
{
	SET_SUBSERIES("Ejector")

public:
	EjectorStarPrinter();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Забрать чек в ретрактор.
	virtual bool retract();
};

//--------------------------------------------------------------------------------
