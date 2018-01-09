/* @file Конфигурация фабрики плагинов. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

//------------------------------------------------------------------------------
QString SDK::Plugin::PluginFactory::mName        = "Watchdogs";
QString SDK::Plugin::PluginFactory::mDescription = "Watchdog driver library.";
QString SDK::Plugin::PluginFactory::mAuthor      = "Cyberplat";
QString SDK::Plugin::PluginFactory::mVersion     = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName  = "watchdogs"; // Название dll/so модуля без расширения

// Первый параметр - название модуля без кавычек
Q_EXPORT_PLUGIN2(watchdogs, SDK::Plugin::PluginFactory)

//------------------------------------------------------------------------------
