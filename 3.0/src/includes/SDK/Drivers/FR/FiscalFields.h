/* @file Фискальные реквизиты). */

#pragma once

namespace SDK { namespace Driver { namespace CAllHardware { namespace FiscalFields
{
	const char FDName[]                 = "fd_name";                      // 1000 (Наименование фискального документа).
	const char UserContact[]            = "user_contact";                 // 1008 (Телефон или электронный адрес покупателя).
	const char PayOffAddress[]          = "payoff_address";               // 1009 (Адрес расчетов).
	const char FDDateTime[]             = "fd_date_time";                 // 1012 (Дата и время ФД).
	const char SerialFRNumber[]         = "serial_fr_number";             // 1013 (Заводской номер ФР).
	const char OFDINN[]                 = "ofd_inn";                      // 1017 (ИНН ОФД).
	const char INN[]                    = "inn";                          // 1018 (ИНН пользователя).
	const char PayOffAmount[]           = "payoff_amount";                // 1020 (Сумма расчета в чеке).
	const char Cashier[]                = "cashier";                      // 1021 (Кассир).
	const char UnitName[]               = "unit_name";                    // 1030 (Наименование товара).
	const char AutomaticNumber[]        = "automatic_number";             // 1036 (Номер автомата).
	const char RNM[]                    = "rnm";                          // 1037 (Регистрационный номер ККТ).
	const char SessionNumber[]          = "session_number";               // 1038 (Номер смены).
	const char FDNumber[]               = "fd_number";                    // 1040 (Номер ФД).
	const char SerialFSNumber[]         = "serial_fs_number";             // 1041 (Заводской номер ФН).
	const char DocumentNumber[]         = "document_number";              // 1042 (Номер чека за смену).
	const char OFDName[]                = "ofd_name";                     // 1046 (Наименование ОФД).
	const char LegalOwner[]             = "legal_owner";                  // 1048 (Наименование юр. лица владельца).
	const char PayOffType[]             = "payoff_type";                  // 1054 (Признак расчета).
	const char TaxSystem[]              = "tax_system";                   // 1055 (СНО на платеже).
	const char FTSURL[]                 = "fts_url";                      // 1060 (Адрес сайта ФНС).
	const char TaxSystemsReg[]          = "tax_systems_reg";              // 1062 (СНО из итогов регистрации).
	const char ProcessingPhone[]        = "processing_phone";             // 1074 (Телефон оператора по приему платежей).
	const char FDSign[]                 = "fd_sign";                      // 1077 (Фискальный признак документа).
	const char OFDNotSentFDQuantity[]   = "ofd_not_sent_fd_quantity";     // 1097 (Количество непереданных ФД).
	const char OFDNotSentFDDateTime[]   = "ofd_not_sent_fd_date_time";    // 1098 (Дата и время первого из непереданных ФД).
	const char ReregistrationCause[]    = "reregistration_cause";         // 1101 (Код причины перерегистрации).
	const char FDForSessionTotal[]      = "fd_for_session_total";         // 1111 (Общее количество ФД за смену).
	const char SenderMail[]             = "sender_mail";                  // 1117 (Электронная почта отправителя чека).
	const char FiscalsForSessionTotal[] = "fiscals_for_session_total";    // 1118 (Количество кассовых чеков (БСО) за смену).
	const char PayOffPlace[]            = "payoff_place";                 // 1187 (Место расчетов).
	const char ModelVersion[]           = "model_version";                // 1188 (Версия модели ККТ).
	const char FFDFR[]                  = "ffd_fr";                       // 1189 (Версия ФФД ФР).
	const char FFDFS[]                  = "ffd_fs";                       // 1190 (Версия ФФД ФН).
	const char VATRate[]                = "vat_rate";                     // 1199 (Ставка НДС).
	const char CashierINN[]             = "cashier_inn";                  // 1203 (ИНН кассира).
	const char OFDURL[]                 = "ofd_url";                      // 1208 (Адрес сайта для получения чека).
	const char FFD[]                    = "ffd";                          // 1209 (Версия ФФД).

	// Данные оператора перевода
	const char TransferOperatorAddress[] = "transfer_operator_address";    // 1005 (Адрес оператора перевода).
	const char TransferOperatorINN[]     = "transfer_operator_inn";        // 1016 (ИНН оператора перевода).
	const char TransferOperatorName[]    = "transfer_operator_name";       // 1026 (Наименование оператора перевода).
	const char TransferOperatorPhone[]   = "transfer_operator_phone";      // 1075 (Телефон оператора перевода).

	// Данные поставщика
	const char ProviderPhone[]   = "provider_phone";     // 1171 (Телефон поставщика).
	const char ProviderINN[]     = "provider_inn";       // 1226 (ИНН поставщика).

	// Данные платежного агента
	const char AgentOperation[]  = "agent_operation";    // 1044 (Операция платежного агента).
	const char AgentFlagsReg[]   = "agent_flags_reg";    // 1057 (Признак(и) платежного агента из итогов регистрации и на платеже).
	const char AgentPhone[]      = "agent_phone";        // 1073 (Телефон платежного агента).
	const char AgentFlag[]       = "agent_flag";         // 1222 (Признак платежного агента на платеже).

	// Статусы
	const char FSExpiredStatus[]    = "fs_expired_status";           // 1050 (Признак исчерпания ресурса ФН).
	const char FSNeedChangeStatus[] = "fs_need_change_status";       // 1051 (Признак необходимости срочной замены ФН).
	const char FSMemoryEnd[]        = "fs_memory_end_status";        // 1052 (Признак заполнения памяти ФН).
	const char OFDNoConnection[]    = "ofd_no_connection_status";    // 1053 (Признак превышения времени ожидания ответа ОФД).

	// Режимы работы
	const char AutomaticMode[]      = "automatic_mode";          // 1001 (Признак автоматического режима).
	const char AutonomousMode[]     = "autonomous_mode";         // 1002 (Признак автономного режима).
	const char EncryptionMode[]     = "encryption_mode";         // 1056 (Признак шифрования).
	const char InternetMode[]       = "internet_mode";           // 1108 (Признак работы с интернет (без принтера).
	const char ServiceAreaMode[]    = "service_area_mode";       // 1109 (Признак применения в сфере услуг).
	const char FixedReportingMode[] = "fixed_reporting_mode";    // 1110 (Признак работы с бланками строгой отчетности (БСО).
	const char LotteryMode[]        = "lottery_mode";            // 1126 (Признак проведения лотереи).
	const char GamblingMode[]       = "gambling_mode";           // 1193 (Признак проведения азартных игр).
	const char ExcisableUnitMode[]  = "excisable_unit_mode";     // 1207 (Признак торговли подакцизными товарами).
	const char InAutomateMode[]     = "in_automate_mode";        // 1221 (Признак установки в автомате).

	// Предмет расчета (на конкретную продажу)
	const char PayOffSubjectQuantity[]   = "payoff_subject_quantity";       // 1023 (Количество предмета расчета).
	const char PayOffSubjectAmount[]     = "payoff_subject_amount";         // 1043 (Стоимость предмета расчета).
	const char PayOffSubject[]           = "payoff_subject";                // 1059 (Предмет расчета).
	const char PayOffSubjectUnitPrice[]  = "payoff_subject_unit_price";     // 1079 (Цена за единицу предмета расчета с учетом скидок и наценок).
	const char PayOffSubjectTaxAmount[]  = "payoff_subject_tax_amount";     // 1200 (Сумма НДС за предмет расчета).
	const char PayOffSubjectType[]       = "payoff_subject_type";           // 1212 (Признак предмета расчета).
	const char PayOffSubjectMethodType[] = "payoff_subject_method_type";    // 1214 (Признак способа расчета).

	// Суммы по способу расчета (на весь чек)
	const char CashFiscalTotal[]         = "cash_fiscal_total";             // 1031 (Сумма по чеку (БСО) наличными).
	const char CardFiscalTotal[]         = "card_fiscal_total";             // 1081 (Сумма по чеку (БСО) электронными).
	const char PrePaymentFiscalTotal[]   = "prepayment_fiscal_total";       // 1215 (Сумма по чеку (БСО) предоплатой (зачетом аванса)).
	const char PostPaymentFiscalTotal[]  = "postpayment_fiscal_total";      // 1216 (Сумма по чеку (БСО) постоплатой (в кредит)).
	const char CounterOfferFiscalTotal[] = "counter_offer_fiscal_total";    // 1217 (Сумма по чеку (БСО) встречным предоставлением).

	// Налоги (на весь чек)
	const char TaxAmount02[] = "tax_amount_02";    // 1102 (Сумма НДС чека по ставке 18(20)%).
	const char TaxAmount03[] = "tax_amount_03";    // 1103 (Сумма НДС чека по ставке 10%).
	const char TaxAmount04[] = "tax_amount_04";    // 1104 (Сумма расчета по чеку с НДС по ставке 0%).
	const char TaxAmount05[] = "tax_amount_05";    // 1105 (Сумма расчета по чеку без НДС).
	const char TaxAmount06[] = "tax_amount_06";    // 1106 (Сумма НДС чека по расчетной ставке 18/118 (20/120)).
	const char TaxAmount07[] = "tax_amount_07";    // 1107 (Сумма НДС чека по расчетной ставке 10/110).

	// Значения.
	namespace Values
	{
		/// ФФД маркер отсутствия данных.
		const char NoData[] = "none";
	}

}}}}

namespace CFiscalSDK = SDK::Driver::CAllHardware::FiscalFields;

//--------------------------------------------------------------------------------
