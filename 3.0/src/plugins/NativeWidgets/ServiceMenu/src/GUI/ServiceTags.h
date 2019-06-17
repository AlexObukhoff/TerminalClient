/* @file Константы аргументов сервисных команд. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

//------------------------------------------------------------------------
namespace CServiceTags
{
	const QString Id                       = "#id";

	const QString Error                    = "#error_message";

	const QString Connection               = "#connection";
	const QString ConnectionStatus         = "#connection_status";
	const QString ConnectionType           = "#connection_type";
	const QString ProxyType                = "#proxy_type";
	const QString ProxyAddress             = "#proxy_address";
	const QString ProxyPort                = "#proxy_port";
	const QString ProxyUser                = "#proxy_user";
	const QString ProxyPassword            = "#proxy_password";
	const QString CheckInterval            = "#check_interval";
	const QString CheckHost                = "#check_host";

	const QString Validator                = "#validator";
	const QString Printer                  = "#printer";
	const QString Watchdog                 = "#watchdog";
	const QString Modem                    = "#modem";
	const QString ValidatorStatus          = "#validator_status";
	const QString PrinterStatus            = "#printer_status";
	const QString FiscalPrinter            = "#fiscal_printer";
	const QString WatchdogStatus           = "#watchdog_status";
	const QString ModemStatus              = "#modem_status";
	const QString DeviceType               = "#device_type";

	namespace UserRole
	{
		const QString RoleAdministrator = "Administrator";
		const QString RoleTechnician = "Technician";
		const QString RoleCollector = "Collector";
	}

	namespace PrinterState
	{
		const QString Printer                = "#printer";
		const QString IsFiscal               = "#is_fiscal";
		const QString Fiscal                 = "#fiscal";
		const QString Reports                = "#reports";
		const QString XReport                = "#x_report";
	}

	namespace Payment
	{
		const QString Number                 = "payment_number";
		const QString Account                = "payment_account";
		const QString PaymentStatusName      = "payment_status_name";
		const QString ProviderFields         = "provider_fields";
	}

	const QString LastPaymentDate          = "#last_payment_date";
	const QString LastProcessedPaymentDate = "#last_processed_payment_date";
	const QString SuccessfulPaymentCount   = "#successful_payment_count";
	const QString FailedPaymentCount       = "#failed_payment_count";
	const QString LastPaymentsInfo         = "#last_payments_info";
	const QString ShowOnlyLastPayments     = "#show_only_last_payments";

	const QString NoteCount                = "#note_count";
	const QString CoinCount                = "#coin_count";
	const QString CashAmount               = "#cash_amount";

	const QString EncashmentID             = "ENCASHMENT_NUMBER";
	const QString EncashmentDate           = "ENCASHMENT_END_DATE";
	const QString LastEncashmentDate       = "#last_encashment_date";

	const QString AutoEncashmentEnabled    = "#auto_encashment";
	const QString AuthorizationEnabled     = "#authorization";
	const QString TerminalLocked           = "#terminal_locked";
	const QString SoftwareVersion          = "#software_version";
	const QString TerminalNumber           = "#terminal_number";

	const QString TargetId                 = "#target_id";
	const QString Login                    = "#login";
	const QString Password                 = "#password";
	const QString CountryId                = "#country_id";
	const QString Url                      = "#url";
	const QString AP                       = "#ap";
	const QString SD                       = "#sd";
	const QString OP                       = "#op";
	const QString KeysGateways             = "#keys_gateways";
	const QString ServerPublicKey          = "#server_public_key";
	const QString KeyPairNumber            = "#key_pair_number";
	const QString KeyPairDescription       = "#key_pair_description";
}

//------------------------------------------------------------------------
