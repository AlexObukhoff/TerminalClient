/* @file Окно редактирования купюр в диспенсере. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IFundsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Project
#include "Backend/ServiceMenuBackend.h"
#include "Backend/HardwareManager.h"
#include "DispenserServiceWindow.h"

//------------------------------------------------------------------------
DispenserServiceWindow::DispenserServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent)
	: QFrame(aParent),
	  ServiceWindowBase(aBackend)
{
	setupUi(this);
}

//------------------------------------------------------------------------
bool DispenserServiceWindow::activate()
{
	lwCashUnits->clear();

	PPSDK::TCashUnitsState cashUnitState = mBackend->getCore()->getFundsService()->getDispenser()->getCashUnitsState();
	
	if (cashUnitState.isEmpty())
	{
		return false;
	}

	foreach(QString device, cashUnitState.keys())
	{
		QVariantMap config = mBackend->getHardwareManager()->getDeviceConfiguration(device);
		
		foreach(PPSDK::SCashUnit cashUnit, cashUnitState.value(device))
		{
				QListWidgetItem * item = new QListWidgetItem(QString("%1:%2 -> %3 x %4 %5 = %6%7")
					.arg(config.value("system_name").toString())
					.arg(config.value("model_name").toString())
					.arg(cashUnit.nominal)
					.arg(cashUnit.count)
					.arg(tr("#pts"))
					.arg(cashUnit.nominal * cashUnit.count)
					.arg(cashUnit.currencyName), lwCashUnits);
				
				lwCashUnits->addItem(item);
		}
	}

	return true;
}

//------------------------------------------------------------------------
bool DispenserServiceWindow::deactivate()
{
	return true;
}

//------------------------------------------------------------------------
bool DispenserServiceWindow::initialize()
{
	return true;
}

//------------------------------------------------------------------------
bool DispenserServiceWindow::shutdown()
{
	return true;
}

//------------------------------------------------------------------------
