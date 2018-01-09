/* @file Шаблон конфигурации фабрики плагинов. */

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

//------------------------------------------------------------------------------
#error Измени описание плагина!
QString SDK::Plugin::PluginFactory::mName        = "My plugin";
QString SDK::Plugin::PluginFactory::mDescription = "Rock'n'Rolling plugin";
QString SDK::Plugin::PluginFactory::mAuthor      = "Evilcom Sofware";
QString SDK::Plugin::PluginFactory::mVersion     = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName  = "MyPlugin"; // Название dll/so модуля без расширения

// Первый параметр - название модуля без кавычек
#error Измени название плагина!
Q_EXPORT_PLUGIN2(MyPlugin, SDK::Plugin::PluginFactory)

//------------------------------------------------------------------------------
