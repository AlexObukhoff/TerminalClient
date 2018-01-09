/* @file Конфигурация фабрики. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

//------------------------------------------------------------------------------
QString SDK::Plugin::PluginFactory::mName        = "Webkit graphics backend";
QString SDK::Plugin::PluginFactory::mDescription = "Webkit based graphics backend for html widgets (with webkit plugins support)";
QString SDK::Plugin::PluginFactory::mAuthor      = "Cyberplat";
QString SDK::Plugin::PluginFactory::mVersion     = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName  = "webkit_backend";

Q_EXPORT_PLUGIN2(webkit_backend, SDK::Plugin::PluginFactory)

//------------------------------------------------------------------------------
