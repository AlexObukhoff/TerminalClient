/* @file Конфигурация фабрики. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

//------------------------------------------------------------------------------
QString SDK::Plugin::PluginFactory::mName        = "Native QWidget graphics backend";
QString SDK::Plugin::PluginFactory::mDescription = "Graphics backend for widgets implemented as binary plugins";
QString SDK::Plugin::PluginFactory::mAuthor      = "Cyberplat";
QString SDK::Plugin::PluginFactory::mVersion     = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName  = "native_backend";

Q_EXPORT_PLUGIN2(native_backend, SDK::Plugin::PluginFactory)

//------------------------------------------------------------------------------
