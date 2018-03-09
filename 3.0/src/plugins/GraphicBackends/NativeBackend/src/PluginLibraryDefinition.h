/* @file ������������ ������� ��������. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

class NativeBackendPluginFactory : public SDK::Plugin::PluginFactory
{
	Q_OBJECT
	Q_INTERFACES(SDK::Plugin::IPluginFactory)
	Q_PLUGIN_METADATA(IID "Cyberplat.Graphics.Backend.Native")
};

//------------------------------------------------------------------------------
