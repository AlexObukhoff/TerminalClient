/* @file Плагин драйвера GSM модема. */

#include "Hardware/Plugins/CommonParameters.h"
#include "../../../../modules/Hardware/Modems/src/ATModem/ATGSMModem.h"

using namespace SDK::Plugin;

//------------------------------------------------------------------------------
TParameterList EnumParameters()
{
	return createNamedList<ATGSMModem>("GSM AT compatible modem");
}

// Регистрация плагина.
REGISTER_DRIVER("Modem", ATGSMModem, &EnumParameters);

//-----------------------------------------------------------------------------
