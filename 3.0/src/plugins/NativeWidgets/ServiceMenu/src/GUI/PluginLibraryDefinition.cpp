/* @file Конфигурация фабрики плагинов. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

//--------------------------------------------------------------------------
QString SDK::Plugin::PluginFactory::mName        = "Service menu native widget";
QString SDK::Plugin::PluginFactory::mDescription = "Service menu.";
QString SDK::Plugin::PluginFactory::mAuthor      = "Cyberplat";
QString SDK::Plugin::PluginFactory::mVersion     = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName  = "service_menu"; // Название dll/so модуля без расширения

// Первый параметр - название модуля без кавычек
Q_EXPORT_PLUGIN2(service_menu, SDK::Plugin::PluginFactory)

//--------------------------------------------------------------------------
