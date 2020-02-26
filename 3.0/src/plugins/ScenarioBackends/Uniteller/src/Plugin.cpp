/* @file Плагин виртуального драйвера Uniteller. */

// Plugin SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/InteractionTypes.h>

// Modules
#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Plugins/DevicePluginBase.h"
#include "Hardware/Plugins/CommonParameters.h"

// Project
#include "UnitellerDevice.h"

using namespace SDK::Driver;
using namespace SDK::Plugin;

//------------------------------------------------------------------------------
static IPlugin * CreatePlugin(IEnvironment * aEnvironment, const QString & aInstancePath)
{
	auto plugin = new DevicePluginBase<UnitellerDevice>(Uniteller::ModelName, aEnvironment, aInstancePath);

	plugin->setCore(dynamic_cast<SDK::PaymentProcessor::ICore *>(aEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore)));
	plugin->setLog(aEnvironment->getLog(Uniteller::LogName));

	return plugin;
}

TParameterList defaultParameters()
{
	return TParameterList()
		<< SPluginParameter(CHardwareSDK::ModelName, false, CPPT::ModelName, QString(), Uniteller::ModelName, QStringList() << Uniteller::ModelName, true);
}

// Регистрация плагина.
REGISTER_DRIVER_WITH_PARAMETERS(UnitellerDevice, &CreatePlugin, &defaultParameters)

//--------------------------------------------------------------------------------------
