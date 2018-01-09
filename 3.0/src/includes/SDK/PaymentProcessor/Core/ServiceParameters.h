/* @file Имена параметров сервисов. */

#pragma once

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
namespace CServiceParameters
{
	namespace Networking
	{
		const char SimBalance[]  = "sim_balance";  /// Баланс на сим-карте.
		const char SignalLevel[] = "signal_level"; /// Уровень сигнала.
		const char Provider[]    = "provider";     /// Имя оператора.
	}

	namespace Funds
	{
		const char RejectCount[] = "reject_count"; /// Число непринятых купюр.
	}

	namespace Printing
	{
		const char ReceiptCount[]    = "receipts_printed"; /// Число напечатанных чеков.
		const char SessionStatus[]   = "session_status";   /// Сессия ФРа: открыта/закрыта.
		const char ZReportCount[]    = "z_report_count";   /// Число имеющихся Z-отчетов.
	}

	namespace Payment
	{
		const char UnprocessedPaymentCount[] = "unprocessed_payments"; /// Число необработаных платежей
		const char PaymentsPerDay[]          = "payments_per_day";     /// Число платежей за день
	}

	namespace Terminal
	{
		const char RestartCount[] = "restart_count"; /// Число запусков ПО.
	}
}

//------------------------------------------------------------------------------
}}

