/* @file Конфигурация фабрики плагинов. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

class BillAcceptorPluginFactory : public SDK::Plugin::PluginFactory
{
	Q_OBJECT
	Q_INTERFACES(SDK::Plugin::IPluginFactory)
	Q_PLUGIN_METADATA(IID "com.cyberplat.bill_acceptors")
};

//------------------------------------------------------------------------------
