/* @file Типы чеков по умолчанию. */

#pragma once

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
namespace CReceiptType
{
	const char Payment[] = "payment";
	const char Error[] = "error";
	const char Balance[] = "balance";
	const char Encashment[] = "encashment";
	const char ZReport[] = "z_report";
	const char ZReportFull[] = "z_report_full";
	const char XReport[] = "x_report";
	const char Test[] = "test";
	const char DispenserEncashment[] = "dispense_encachment";
	const char DispenserBalance[] = "dispense_balance";
}

//---------------------------------------------------------------------------
}} // SDK::PaymentProcessor

