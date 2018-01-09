/* @file Имена параметров в БД. */

#pragma once

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
namespace CDatabaseConstants
{
	/// Имена обязательных устройств.
	namespace Devices
	{
		const char Terminal[] = "Terminal";
	}

	/// Имена параметров в БД.
	namespace Parameters
	{
		/// Общие параметры.
		const char DeviceName[] = "device_name";         /// Содержит имя устройства.
		const char DeviceInfo[] = "device_info";         /// Содержит различную информацию о устройстве.

		/// Параметры принтеров.
		const char ReceiptCount[] = "receipt_count";     /// Счётчик напечатанных чеков.

		/// Параметры модемов.
		const char BalanceLevel[] = "balance_level";     /// Баланс сим-карты.
		const char SignalLevel[] = "signal_level";       /// Уровень сигнала модема.
		const char ConnectionName[] = "connection_name"; /// Название модемного соединения.
		const char LastCheckBalanceTime[] = "last_check_balance_time"; /// Время последней проверки баланса и уровня сигнала

		/// Параметры купюроприемника.
		const char RejectCount[] = "reject_count";

		/// Параметры терминала.
		const char DisabledParam[] = "disabled";          /// Заблокирован или нет терминал.
		const char LastUpdateTime[] = "last_update_time"; /// Время последнего скачативания конфигов.
		const char LaunchCount[] = "launch_count";        /// Число запусков ПО.
		const char LastStartDate[] = "last_start_date";   /// Время последнего запуска.
		const char Configuration[] = "configuration";     /// Конфигурация ПО.
		const char OperationSystem[] = "operation_system";/// Версия операционной системы.
		const char DisplayResolution[] = "display_resolution"; /// Разрешение экрана.

		/// Параметры диспенсера
		const char CashUnits[] = "cash_units";            /// Описание содержимого диспенсера по кассетам
	}
}

//---------------------------------------------------------------------------
}}