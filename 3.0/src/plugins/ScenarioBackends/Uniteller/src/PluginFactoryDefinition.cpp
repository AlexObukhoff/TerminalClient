/* @file Конфигурация фабрики. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

//------------------------------------------------------------------------------
QString SDK::Plugin::PluginFactory::mName        = "uniteller";
QString SDK::Plugin::PluginFactory::mDescription = "Native scenario for Uniteller";
QString SDK::Plugin::PluginFactory::mAuthor      = "Cyberplat";
QString SDK::Plugin::PluginFactory::mVersion     = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName  = "uniteller";

Q_EXPORT_PLUGIN2(uniteller, SDK::Plugin::PluginFactory)

//------------------------------------------------------------------------------
