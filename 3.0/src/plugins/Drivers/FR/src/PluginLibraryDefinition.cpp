/* @file Конфигурация фабрики плагинов. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

//------------------------------------------------------------------------------
QString SDK::Plugin::PluginFactory::mName        = "FR";
QString SDK::Plugin::PluginFactory::mDescription = "FR driver library.";
QString SDK::Plugin::PluginFactory::mAuthor      = "Cyberplat";
QString SDK::Plugin::PluginFactory::mVersion     = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName  = "FR"; // Название dll/so модуля без расширения

// Первый параметр - название модуля без кавычек
Q_EXPORT_PLUGIN2(FR, SDK::Plugin::PluginFactory)

//------------------------------------------------------------------------------
