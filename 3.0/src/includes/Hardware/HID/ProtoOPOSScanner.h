/* @file Прото-OPOS-сканер. */

#pragma once

#include "ProtoHID.h"

//--------------------------------------------------------------------------------
class ProtoOPOSScanner : public ProtoHID
{
	Q_OBJECT

protected slots:
	/// Включает/выключает устройство на чтение штрих-кодов. Пикать все равно будет.
	virtual bool enable(bool /*aEnabled*/) { return true; };

	/// Вызывается по приходу данных от сканера.
	void onGotData(const QString & /*aName*/, int /*aArgumentsCount*/, void * /*aArgumentsValues*/) {};
};

//--------------------------------------------------------------------------------
