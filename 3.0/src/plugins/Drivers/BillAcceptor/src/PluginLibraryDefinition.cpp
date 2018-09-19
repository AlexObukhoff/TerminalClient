/* @file Конфигурация фабрики плагинов. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

//------------------------------------------------------------------------------
QString SDK::Plugin::PluginFactory::mName        = "BillAcceptor";
QString SDK::Plugin::PluginFactory::mDescription = "BillAcceptor driver library, CCNet protocol";
QString SDK::Plugin::PluginFactory::mAuthor      = "Cyberplat";
QString SDK::Plugin::PluginFactory::mVersion     = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName  = "bill_acceptors"; // Название dll/so модуля без расширения

//------------------------------------------------------------------------------
