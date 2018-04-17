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

// Время автозакрытия смены.
SDK::Plugin::SPluginParameter setSessionOpeningTime();

// Печатать/не печатать.
SDK::Plugin::SPluginParameter setNotPrinting();

// Модель подключенного принтера для Казначея.
SDK::Plugin::SPluginParameter setPaymasterPrinterModel();

//------------------------------------------------------------------------------
