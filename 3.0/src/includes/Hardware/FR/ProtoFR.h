/* @file Прото-ФР. */

#pragma once

// SDK
#include <SDK/Drivers/IFiscalPrinter.h>

// Modules
#include "Hardware/Common/ProtoDevice.h"

//--------------------------------------------------------------------------------
class ProtoFR: public ProtoDevice, public MetaDevice<SDK::Driver::IFiscalPrinter>
{
	Q_OBJECT

	SET_DEVICE_TYPE(FiscalRegistrator)

signals :
	/// Данные о закрытой сессии.
	void FRSessionClosed(const QVariantMap & aOutData);

protected slots:
	/// Выполнить Z-отчет.
	virtual void onExecZReport() {}
};

//--------------------------------------------------------------------------------
