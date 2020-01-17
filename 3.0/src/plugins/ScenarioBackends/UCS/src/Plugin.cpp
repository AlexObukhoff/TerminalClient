/* @file Плагин виртуального драйвера UCS. */

// Plugin SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/InteractionTypes.h>

// Modules
#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Plugins/DevicePluginBase.h"
#include "Hardware/Plugins/CommonParameters.h"

// Project
#include "UcsDevice.h"

using namespace SDK::Driver;
using namespace SDK::Plugin;

//------------------------------------------------------------------------------
static IPlugin * CreatePlugin(IEnvironment * aEnvironment, const QString & aInstancePath)
{
	auto plugin = new DevicePluginBase<UcsDevice>(Ucs::ModelName, aEnvironment, aInstancePath);

	plugin->setCore(dynamic_cast<SDK::PaymentProcessor::ICore *>(aEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore)));
	plugin->setLog(aEnvironment->getLog(Ucs::LogName));

	return plugin;
}

TParameterList defaultParameters()
{
	return TParameterList()
		<< SPluginParameter(CHardwareSDK::ModelName, false, CPPT::ModelName, QString(), Ucs::ModelName, QStringList() << Ucs::ModelName, true);
}

// Регистрация плагина.
REGISTER_DRIVER_WITH_PARAMETERS(UcsDevice, &CreatePlugin, &defaultParameters)

//--------------------------------------------------------------------------------------
