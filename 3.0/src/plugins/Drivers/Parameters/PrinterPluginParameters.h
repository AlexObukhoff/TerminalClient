/* @file Параметры плагинов для принтеров. */

#pragma once

// Plugin SDK
#include <SDK/Plugins/PluginInitializer.h>

//------------------------------------------------------------------------------
/// Выводить номера страниц на печать - не выводить.
SDK::Plugin::SPluginParameter setPaginationDisabled();

/// Датчик скорого конца бумаги.
SDK::Plugin::SPluginParameter setRemoteSensor(bool aEnabled);

/// Датчик замятия - включен.
SDK::Plugin::SPluginParameter setJamSensorEnabled();

/// Весовые датчики - включены.
SDK::Plugin::SPluginParameter setWeightSensorsEnabled();

/// Размер шрифта.
SDK::Plugin::SPluginParameter setFontSize(int aMin, int aMax, int aDefault, int aInterval);

/// Межстрочный интервал.
SDK::Plugin::SPluginParameter setLineSpacing(int aMin, int aMax, int aDefault, int aInterval, const QString & aOptionalTranslation = "");

/// Коэффициент промотки.
SDK::Plugin::SPluginParameter setFeedingFactor();

/// Петля.
SDK::Plugin::SPluginParameter setLoopEnabled(const QString & aOptionalTranslation = "", bool aNoChange = true);

/// Таймаут для действия с незабранным чеком.
SDK::Plugin::SPluginParameter setLeftReceiptTimeout(bool aZero = false);

/// Действие с незабранным чеком по таймауту.
SDK::Plugin::SPluginParameter setLeftReceiptAction(const QString & aParameter, bool aRetract, bool aPush, const QString aDefault, bool aNoChange = true, const QString & aOptionalTranslation = "");

/// Длина презентации чека.
SDK::Plugin::SPluginParameter setPresentationLength(const QString & aOptionalTranslation = "", int aMin = -1, int aMax = -1);

/// Длина презентации чека.
SDK::Plugin::SPluginParameter setCodepage();

/// Способ формирования фискального чека.
SDK::Plugin::SPluginParameter setFiscalChequeCreation();

/// Обратная промотка.
SDK::Plugin::SPluginParameter setBackFeed(const QString & aOptionalTranslation = "");

//------------------------------------------------------------------------------
