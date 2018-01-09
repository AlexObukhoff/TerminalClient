/* @file Типы данных ФР c эжектором на протоколе АТОЛ. */

#pragma once

//--------------------------------------------------------------------------------
namespace CEjectorAtolFR
{
	const char InitReceipt        = '\x20';    /// Начальное значение параметров эжектора.
	const char SpecialSettingMask = '\xF0';    /// Маска спец. настройки.
	const char PushLastDocument   = '\x40';    /// Выбросить предыдущий документ.

	/// Структура параметров эжектора.
	struct SData
	{
		char receipt;        /// Печать чеков.
		char autoZReport;    /// Автозакрытие смены в ретрактор.
		char ZReport;        /// Закрытие смены.
		char nextMask;       /// Маска для управления печати чеком при непрерывной подаче следующего.

		SData() : receipt(InitReceipt), autoZReport(InitReceipt), ZReport(InitReceipt), nextMask(0) {}    //разнести по моделям
		SData(char aReceipt, char aAutoZReport, char aZReport, char aNextMask) : receipt(aReceipt), autoZReport(aAutoZReport), ZReport(aZReport), nextMask(aNextMask) {}
	};
}

//--------------------------------------------------------------------------------
