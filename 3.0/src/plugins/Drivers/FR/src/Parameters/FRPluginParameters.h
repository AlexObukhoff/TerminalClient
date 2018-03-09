/* @file Параметры плагинов для ФР. */

#pragma once

// Plugin SDK
#include <SDK/Plugins/PluginInitializer.h>

//------------------------------------------------------------------------------
/// Фискальный режим - включен.
SDK::Plugin::SPluginParameter setFiscalModeEnabled();

// Заголовок фискального чека - не трогаем.
SDK::Plugin::SPluginParameter setDocumentCap();

// Разрешение автозакрывать смену на инициализации.
SDK::Plugin::SPluginParameter setAutoCloseSessionAbility();

//------------------------------------------------------------------------------
