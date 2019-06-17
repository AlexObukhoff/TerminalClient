/* @file Конфигурация фабрики. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

//------------------------------------------------------------------------------
QString SDK::Plugin::PluginFactory::mName        = "Migrator 3000";
QString SDK::Plugin::PluginFactory::mDescription = "Native scenario for automatic migration from 2.x.x to 3.x.x version";
QString SDK::Plugin::PluginFactory::mAuthor      = "Cyberplat";
QString SDK::Plugin::PluginFactory::mVersion     = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName  = "migrator3000";

Q_EXPORT_PLUGIN2(migrator3000, SDK::Plugin::PluginFactory)

//------------------------------------------------------------------------------
