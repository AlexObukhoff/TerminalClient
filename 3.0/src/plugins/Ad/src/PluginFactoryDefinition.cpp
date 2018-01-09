/* @file Конфигурация фабрики. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

//------------------------------------------------------------------------------
QString SDK::Plugin::PluginFactory::mName        = "Cyberplat nice advertisement client";
QString SDK::Plugin::PluginFactory::mDescription = "";
QString SDK::Plugin::PluginFactory::mAuthor      = "Cyberplat";
QString SDK::Plugin::PluginFactory::mVersion     = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName  = "ad";

Q_EXPORT_PLUGIN2(ad, SDK::Plugin::PluginFactory)

//------------------------------------------------------------------------------
