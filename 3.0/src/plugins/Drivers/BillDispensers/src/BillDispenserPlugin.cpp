/* @file Плагин c драйверами диспенсеров. */

// Modules
#include "Hardware/Plugins/CommonParameters.h"

// Project
#include "../../../../modules/Hardware/CashDispensers/src/Puloon/PuloonCashDispenser.h"
#include "../../../../modules/Hardware/CashDispensers/src/Suzo/SuzoHopper.h"

using namespace SDK::Plugin;

//------------------------------------------------------------------------------
template <class T>
IPlugin * CreatePlugin(IEnvironment * aEnvironment, const QString & aInstancePath)
{
	return new DevicePluginBase<T>("Puloon dispenser", aEnvironment, aInstancePath);
}

//------------------------------------------------------------------------------
template <>
IPlugin * CreatePlugin<SuzoHopper>(IEnvironment * aEnvironment, const QString & aInstancePath)
{
	return new DevicePluginBase<SuzoHopper>("Suzo hopper", aEnvironment, aInstancePath);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList defaultParameters(const QString & aDefaultName)
{
	return createNamedList<T>(T::getModelList(), aDefaultName);
}

#define COMMON_DISPENSER_PLUGIN(aClassName, aDefaultName) COMMON_DRIVER(aClassName, std::bind(&defaultParameters<aClassName>, aDefaultName))

// Регистрация плагинов.
BEGIN_REGISTER_PLUGIN
	COMMON_DISPENSER_PLUGIN(PuloonLCDM, "Puloon LCDM dispenser")
	COMMON_DISPENSER_PLUGIN(SuzoHopper, "Suzo hopper")
END_REGISTER_PLUGIN

//------------------------------------------------------------------------------
