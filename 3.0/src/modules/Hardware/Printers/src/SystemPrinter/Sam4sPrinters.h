/* @file Принтеры Sam4s на системном драйвере. */

#pragma once

#include "SystemPrinter.h"
#include "Sam4sModelData.h"

//--------------------------------------------------------------------------------
class Sam4s : public SystemPrinter
{
	SET_SERIES("Sam4s")

public:
	Sam4s();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Попытка самоидентификации.
	virtual bool isConnected();
};

//--------------------------------------------------------------------------------
