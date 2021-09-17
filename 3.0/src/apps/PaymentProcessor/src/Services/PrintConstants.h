/* @file Менеджер конфигураций. */

#pragma once

//---------------------------------------------------------------------------
/// Константы для печати чеков.
namespace CPrintConstants
{
	const char * const BankName              = "BANK_NAME";
	const char * const BankBik               = "BANK_BIK";
	const char * const BankInn               = "BANK_INN";
	const char * const BankAddress           = "BANK_ADDRESS";
	const char * const BankPhone             = "BANK_PHONE";
	const char * const DealerAddress         = "DEALER_ADDRESS";
	const char * const DealerBusinessAddress = "DEALER_BUSINESS_ADDRESS";
	const char * const DealerInn             = "DEALER_INN";
	const char * const DealerKbk             = "DEALER_KBK";
	const char * const DealerName            = "DEALER_NAME";
	const char * const DealerPhone           = "DEALER_PHONE";
	const char * const DealerIsBank          = "DEALER_IS_BANK";
	const char * const DealerSupportPhone    = "DEALER_SUPPORT_PHONE";
	const char * const DealerVAT             = "DEALER_NDS";
	const char * const DealerAgentFlag       = "DEALER_AGENT_FLAG";
	const char * const DealerTaxSystem       = "DEALER_SNO";
	const char * const FiscalData            = "FISCAL_DATA";
	const char * const MtRegistrationAddress = "MT_REGISTRATION_ADDRESS";
	const char * const PointAddress          = "POINT_ADDRESS";
	const char * const PointName             = "POINT_NAME";
	const char * const PointExternalID       = "POINT_EXTERNAL_ID";
	const char * const OpBrand               = "OPERATOR_BRAND";
	const char * const OpName                = "OPERATOR_NAME";
	const char * const OpINN                 = "OPERATOR_INN";
	const char * const OpPhone               = "OPERATOR_PHONE";
	const char * const RecipientInn          = "RECIPIENT_INN";
	const char * const RecipientName         = "RECIPIENT_NAME";
	const char * const ServiceType           = "SERVICE_TYPE";
	const char * const TermNumber            = "TERMINAL_NUMBER";
	const char * const Currency              = "CURRENCY";
	const char * const ContractNumber        = "CONTRACT_NUMBER";
	const char * const DateTime              = "DATETIME";
	const char * const ReceiptNumber         = "RECEIPT_NUMBER";
	const char * const NoFiscal              = "NO_FISCAL";

	namespace KKM
	{
		const char * const TaxSystem      = "TAXSYSTEM";               // система налогообложения (СНО)
		const char * const DateTimeStamp  = "KKM_DATETIME_STAMP";      // дата и время получения фискального документа
		const char * const SerialNumber   = "KKM_SERIAL_NUMBER";       // серийный номер фискальника (заводской номер ККТ)
		const char * const RNM            = "KKM_RNM";                 // регистрационный номер ККТ (РНМ)
		const char * const SessionNumber  = "KKM_SESSION_NUMBER";      // номер смены
		const char * const FDSerialNumber = "KKM_FD_SERIAL_NUMBER";    // порядковый номер фискального чека
		const char * const FSNumber       = "KKM_FS_NUMBER";           // заводской номер фискального накопителя
		const char * const FDNumber       = "KKM_FD_NUMBER";           // номер фискального чека
		const char * const FDSign         = "KKM_FD_SIGN";             // фискальный признак данных

		const char * const TaxAmount02 = "TAX_AMOUNT_02";    // сумма НДС чека по ставке 18(20)% (1102)
		const char * const TaxAmount03 = "TAX_AMOUNT_03";    // сумма НДС чека по ставке 10% (1103)
		const char * const TaxAmount04 = "TAX_AMOUNT_04";    // сумма расчета по чеку с НДС по ставке 0% (1104)
		const char * const TaxAmount05 = "TAX_AMOUNT_05";    // сумма расчета по чеку без НДС (1105)
	}

	const char PointAddressExists[] = "point_address_exists";
}

//---------------------------------------------------------------------------
