/* @file Константы фискального регистратора для модуля платежей. */

#pragma once

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
/// Константы фискального регистратора.
namespace CFiscalPrinter
{
	const char Serial[] = "serial";
	const char RNM[] = "rnm";
	const char ZReportNumber[] = "z_report_number";
	const char PaymentCount[] = "payment_count";
	const char NonNullableAmount[] = "non_nullable_amount";
	const char PaymentAmount[] = "payment_amount";
	const char FRDateTime[] = "fr_date_time";
	const char SystemDateTime[] = "system_date_time";
}

}} // namespace SDK::Driver

//--------------------------------------------------------------------------------
