/* @file Конфигурация фабрики плагинов. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

class UnitellerPluginFactory : public SDK::Plugin::PluginFactory
{
	Q_OBJECT
	Q_INTERFACES(SDK::Plugin::IPluginFactory)
	Q_PLUGIN_METADATA(IID "com.cyberplat.uniteller")
};

//------------------------------------------------------------------------------
