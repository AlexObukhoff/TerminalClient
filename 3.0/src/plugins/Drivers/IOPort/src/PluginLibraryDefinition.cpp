/* @file Конфигурация фабрики плагинов. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

//------------------------------------------------------------------------------
QString SDK::Plugin::PluginFactory::mName        = "IO ports";
QString SDK::Plugin::PluginFactory::mDescription = "IO ports driver library (serial, parallel and other).";
QString SDK::Plugin::PluginFactory::mAuthor      = "Cyberplat";
QString SDK::Plugin::PluginFactory::mVersion     = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName  = "ioports"; // Название dll/so модуля без расширения

// Первый параметр - название модуля без кавычек
Q_EXPORT_PLUGIN2(ioports, SDK::Plugin::PluginFactory)

//------------------------------------------------------------------------------
