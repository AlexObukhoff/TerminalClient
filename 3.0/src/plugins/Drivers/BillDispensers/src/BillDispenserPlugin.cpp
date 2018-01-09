/* @file Плагин c драйверами диспенсеров. */

// Modules
#include "Hardware/Plugins/CommonParameters.h"

// Project
#include "../../../../modules/Hardware/CashDispensers/src/PuloonCashDispenser.h"

using namespace SDK::Plugin;

//------------------------------------------------------------------------------
TParameterList DefaultParameters()
{
	return createNamedList<PuloonLCDM>(PuloonLCDM::getModelList(), "Bill dispenser");
}

// Регистрация плагина.
REGISTER_DRIVER("Puloon LCDM dispenser", PuloonLCDM, &DefaultParameters)

//------------------------------------------------------------------------------
