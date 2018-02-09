/* @file Фискальные реквизиты. */

#pragma once

namespace SDK {
namespace Driver {

namespace FiscalFields
{
	const int FDName               = 1000;    // Наименование фискального документа.
	const int AutomaticMode        = 1001;    // Признак автоматического режима.
	const int AutonomousMode       = 1002;    // Признак автономного режима.
	const int UserContact          = 1008;    // Телефон или электронный адрес покупателя.
	const int PayOffAddress        = 1009;    // Адрес расчетов.
	const int FDDateTime           = 1012;    // Дата и время ФД.
	const int SerialFRNumber       = 1013;    // Заводской номер ФР.
	const int INN                  = 1018;    // ИНН пользователя.
	const int PayOffAmount         = 1020;    // Сумма расчета в чеке.
	const int Cashier              = 1021;    // Кассир.
	const int UnitName             = 1030;    // Наименование товара.
	const int AutomaticNumber      = 1036;    // Номер автомата.
	const int RNM                  = 1037;    // Регистрационный номер ККТ.
	const int SessionNumber        = 1038;    // Номер смены.
	const int FDNumber             = 1040;    // Номер ФД.
	const int SerialFSNumber       = 1041;    // Заводской номер ФН.
	const int DocumentNumber       = 1042;    // Номер чека за смену.
	const int OFDName              = 1046;    // Наименование ОФД.
	const int LegalOwner           = 1048;    // Наименование юр. лица владельца.
	const int PayOffType           = 1054;    // Признак расчета.
	const int TaxSystem            = 1055;    // СНО на платеже.
	const int EncryptionMode       = 1056;    // Признак шифрования.
	const int AgentFlagsRegistered = 1057;    // Признак(и) платежного агента из итогов регистрации и на платеже.
	const int FTSURL               = 1060;    // Адрес сайта ФНС.
	const int TaxSystemsRegistered = 1062;    // СНО из итогов регистрации.
	const int FDSign               = 1077;    // Фискальный признак документа.
	const int InternetMode         = 1108;    // Признак работы с интернет (без принтера).
	const int ServiceAreaMode      = 1109;    // Признак применения в сфере услуг.
	const int FixedReportingMode   = 1110;    // Признак работы с бланками строгой отчетности (БСО).
	const int LotteryMode          = 1126;    // Признак проведения лотереи.
	const int PayOffPlace          = 1187;    // Место расчетов.
	const int GamblingMode         = 1193;    // Признак проведения азартных игр.
	const int VATRate              = 1199;    // Ставка НДС.
	const int CashierINN           = 1203;    // ИНН кассира.
	const int ExcisableUnitMode    = 1207;    // Признак торговли подакцизными товарами.
	const int OFDURL               = 1208;    // Адрес сайта для получения чека.
	const int AgentFlag            = 1222;    // Признак платежного агента на платеже.
	const int ProviderINN          = 1226;    // ИНН поставщика.

	// Предмет расчета (на конкретную продажу)
	const int PayOffSubjectQuantity   = 1023;    // Количество предмета расчета.
	const int PayOffSubjectAmount     = 1043;    // Стоимость предмета расчета.
	const int PayOffSubject           = 1059;    // Предмет расчета.
	const int PayOffSubjectUnitPrice  = 1079;    // Цена за единицу предмета расчета с учетом скидок и наценок.
	const int PayOffSubjectTaxAmount  = 1200;    // Cумма НДС за предмет расчета.
	const int PayOffSubjectType       = 1212;    // Признак предмета расчета.
	const int PayOffSubjectMethodType = 1214;    // Признак способа расчета.

	// Суммы по способу расчета (на весь чек)
	const int CashFiscalTotal         = 1031;    // Сумма по чеку (БСО) наличными.
	const int CardFiscalTotal         = 1081;    // Сумма по чеку (БСО) электронными.
	const int PrePaymentFiscalTotal   = 1215;    // Сумма по чеку (БСО) предоплатой (зачетом аванса).
	const int PostPaymentFiscalTotal  = 1216;    // Сумма по чеку (БСО) постоплатой (в кредит).
	const int CounterOfferFiscalTotal = 1217;    // Сумма по чеку (БСО) встречным предоставлением.

	// Налоги (на весь чек)
	const int TaxAmount02 = 1102;    // Сумма НДС чека по ставке 18%.
	const int TaxAmount03 = 1103;    // Сумма НДС чека по ставке 10%.
	const int TaxAmount04 = 1104;    // Сумма расчета по чеку с НДС по ставке 0%.
	const int TaxAmount05 = 1105;    // Сумма расчета по чеку без НДС.
	const int TaxAmount06 = 1106;    // Сумма НДС чека по расчетной ставке 18/118.
	const int TaxAmount07 = 1107;    // Сумма НДС чека по расчетной ставке 10/110.

	// Является ли поле денежным.
	inline bool isMoney(int aField)
	{
		return (aField == PayOffAmount)           ||
		       (aField == PayOffSubjectAmount)    ||
		       (aField == PayOffSubjectUnitPrice) ||
		       (aField == PayOffSubjectTaxAmount) ||
		       (aField == TaxAmount02)            ||
		       (aField == TaxAmount03)            ||
		       (aField == TaxAmount04)            ||
		       (aField == TaxAmount05)            ||
		       (aField == TaxAmount06)            ||
		       (aField == TaxAmount07);
	}
}

}} // namespace SDK::Driver

//--------------------------------------------------------------------------------
