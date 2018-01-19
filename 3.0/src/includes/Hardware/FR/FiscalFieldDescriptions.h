/* @file Описатель фискальных реквизитов, используется в драйверах). */

// SDK
#include <SDK/Drivers/FR/FiscalFields.h>

// Modules
#include "Hardware/Common/Specifications.h"

#pragma once

//---------------------------------------------------------------------------
namespace CHardware { namespace FiscalFields
{
	const char FDName[]                  = "fd_name";                       // 1000 (Наименование фискального документа).
	const char AutomaticMode[]           = "automatic_mode";                // 1001 (Признак автоматического режима).
	const char AutonomousMode[]          = "autonomous_mode";               // 1002 (Признак автономного режима).
	const char UserContact[]             = "user_contact";                  // 1008 (Телефон или электронный адрес покупателя).
	const char PayOffAddress[]           = "payoff_address";                // 1009 (Адрес расчетов).
	const char FDDateTime[]              = "fd_date_time";                  // 1012 (Дата и время ФД).
	const char SerialFRNumber[]          = "serial_fr_number";              // 1013 (Заводской номер ФР).
	const char INN[]                     = "inn";                           // 1018 (ИНН пользователя).
	const char PayOffAmount[]            = "payoff_amount";                 // 1020 (Сумма расчета в чеке).
	const char Cashier[]                 = "cashier";                       // 1021 (Кассир).
	const char PayOffSubjectQuantity[]   = "payoff_subject_quantity";       // 1023 (Количество предмета расчета).
	const char UnitName[]                = "unit_name";                     // 1030 (Наименование товара).
	const char AutomaticNumber[]         = "automatic_number";              // 1036 (Номер автомата).
	const char RNM[]                     = "rnm";                           // 1037 (Регистрационный номер ККТ).
	const char SessionNumber[]           = "session_number";                // 1038 (Номер смены).
	const char FDNumber[]                = "fd_number";                     // 1040 (Номер ФД).
	const char SerialFSNumber[]          = "serial_fs_number";              // 1041 (Заводской номер ФН).
	const char DocumentNumber[]          = "document_number";               // 1042 (Номер чека за смену).
	const char PayOffSubjectAmount[]     = "payoff_subject_amount";         // 1043 (Стоимость предмета расчета).
	const char OFDName[]                 = "ofd_name";                      // 1046 (Наименование ОФД).
	const char LegalOwner[]              = "legal_owner";                   // 1048 (Наименование юр. лица владельца).
	const char PayOffType[]              = "payoff_type";                   // 1054 (Признак расчета).
	const char TaxSystem[]               = "tax_system";                    // 1055 (СНО на платеже).
	const char EncryptionMode[]          = "encryption_mode";               // 1056 (Признак шифрования).
	const char AgentFlagsRegistered[]    = "agent_flags_registered";        // 1057 (Признак(и) платежного агента из итогов регистрации).
	const char PayOffSubject[]           = "payoff_subject";                // 1059 (Предмет расчета).
	const char FTSURL[]                  = "fts_url";                       // 1060 (Адрес сайта ФНС).
	const char TaxSystemsRegistered[]    = "tax_systems_registered";        // 1062 (СНО из итогов регистрации).
	const char FDSign[]                  = "fd_sign";                       // 1077 (Фискальный признак документа).
	const char PayOffSubjectUnitPrice[]  = "payoff_subject_unit_price";     // 1079 (Цена за единицу предмета расчета с учетом скидок и наценок).

	const char CashFiscalTotal[]         = "cash_fiscal_total";             // 1031 (Сумма по чеку (БСО) наличными).
	const char CardFiscalTotal[]         = "card_fiscal_total";             // 1081 (Сумма по чеку (БСО) электронными).
	const char PrePaymentFiscalTotal[]   = "prepayment_fiscal_total";       // 1215 (Сумма по чеку (БСО) предоплатой (зачетом аванса).
	const char PostPaymentFiscalTotal[]  = "postpayment_fiscal_total";      // 1216 (Сумма по чеку (БСО) постоплатой (в кредит).
	const char CounterOfferFiscalTotal[] = "counter_offer_fiscal_total";    // 1217 (Сумма по чеку (БСО) встречным предоставлением).

	const char TaxAmount02[] = "tax_amount_02";    // 1102 (Сумма НДС чека по ставке 18%).
	const char TaxAmount03[] = "tax_amount_03";    // 1103 (Сумма НДС чека по ставке 10%).
	const char TaxAmount04[] = "tax_amount_04";    // 1104 (Сумма расчета по чеку с НДС по ставке 0%).
	const char TaxAmount05[] = "tax_amount_05";    // 1105 (Сумма расчета по чеку без НДС).
	const char TaxAmount06[] = "tax_amount_06";    // 1106 (Сумма НДС чека по расчетной ставке 18/118).
	const char TaxAmount07[] = "tax_amount_07";    // 1107 (Сумма НДС чека по расчетной ставке 10/110).

	const char InternetMode[]       = "internet_mode";           // 1108 (Признак работы с интернет (без принтера).
	const char ServiceAreaMode[]    = "service_area_mode";       // 1109 (Признак применения в сфере услуг).
	const char FixedReportingMode[] = "fixed_reporting_mode";    // 1110 (Признак работы с бланками строгой отчетности (БСО).
	const char LotteryMode[]        = "lottery_mode";            // 1126 (Признак проведения лотереи).
	const char PayOffPlace[]        = "payoff_place";            // 1187 (Место расчетов).
	const char GamblingMode[]       = "gambling_mode";           // 1193 (Признак проведения азартных игр).
	const char VATRate[]            = "vat_rate";                // 1199 (Ставка НДС).
	const char CashierINN[]         = "cashier_inn";             // 1203 (ИНН кассира).
	const char ExcisableUnitMode[]  = "excisable_unit_mode";     // 1207 (Признак торговли подакцизными товарами).
	const char OFDURL[]             = "ofd_url";                 // 1208 (Адрес сайта для получения чека).
	const char PayOffSubjectType[]  = "payoff_subject_type";     // 1212 (Признак предмета расчета).
	const char PayOffMethodType[]   = "payoff_method_type";      // 1214 (Признак способа расчета).
	const char AgentFlag[]          = "agent_flag";              // 1222 (Признак платежного агента на платеже).

}}    // namespace CHardware::FiscalFields

//---------------------------------------------------------------------------
namespace CFR { namespace FiscalFields
{
	// Типы данных.
	namespace ETypes
	{
		enum Enum
		{
			None = 0,
			String,
			Byte,
			ByteArray,
			UnixTime,
			VLN,
			FVLN,
			STLV,
			UINT32,
			UINT16
		};
	};

	namespace Types
	{
		struct SData
		{
			int minSize;
			bool fixSize;
			QString description;

			SData(): minSize(0), fixSize(false) {}
			SData(int aMinSize, const QString & aDescription, bool aFixSize = false): minSize(aMinSize), fixSize(aFixSize), description(aDescription) {}
		};

		#define ADD_FISCAL_TYPE(aType, aMinSize, ...) append(ETypes::aType, SData(aMinSize, #aType, __VA_ARGS__))

		class CData: public CSpecification<ETypes::Enum, SData>
		{
		public:
			CData()
			{
				ADD_FISCAL_TYPE(String,    1);
				ADD_FISCAL_TYPE(Byte,      1, true);
				ADD_FISCAL_TYPE(ByteArray, 1);
				ADD_FISCAL_TYPE(UnixTime,  4, true);
				ADD_FISCAL_TYPE(VLN,       1);
				ADD_FISCAL_TYPE(FVLN,      2);
				ADD_FISCAL_TYPE(STLV,      5);
				ADD_FISCAL_TYPE(UINT32,    4, true);
				ADD_FISCAL_TYPE(UINT16,    2, true);
			}
		};

		static CData Data;
	}

	//---------------------------------------------------------------------------
	// Обязательность параметра
	namespace ERequired
	{
		enum Enum
		{
			No = 0,
			PM,
			Yes
		};
	}

	//---------------------------------------------------------------------------
	// Структура описателя.
	struct SData
	{
		ETypes::Enum type;
		QString description;
		ERequired::Enum required;

		SData(): type(ETypes::None), required(ERequired::No) {}
		SData(ETypes::Enum aType, const QString & aDescription): type(aType), description(aDescription), required(ERequired::No) {}
	};

	//---------------------------------------------------------------------------
	class CData: public CSpecification<int, SData>
	{
	public:
		CData()
		{
			#define ADD_FISCAL_FIELD(aName, aType) append(SDK::Driver::FiscalFields::aName, SData(ETypes::aType, CHardware::FiscalFields::aName))

			ADD_FISCAL_FIELD(FDName,                 String);       // 1000 (Наименование фискального документа).
			ADD_FISCAL_FIELD(AutomaticMode,          Byte);         // 1001 (Признак автоматического режима).
			ADD_FISCAL_FIELD(AutonomousMode,         Byte);         // 1002 (Признак автономного режима).
			ADD_FISCAL_FIELD(UserContact,            String);       // 1008 (Телефон или электронный адрес покупателя).
			ADD_FISCAL_FIELD(PayOffAddress,          String);       // 1009 (Адрес расчетов).
			ADD_FISCAL_FIELD(FDDateTime,             UnixTime);     // 1012 (Дата и время ФД).
			ADD_FISCAL_FIELD(SerialFRNumber,         String);       // 1013 (Заводской номер ФР).
			ADD_FISCAL_FIELD(INN,                    String);       // 1018 (ИНН пользователя).
			ADD_FISCAL_FIELD(PayOffAmount,           VLN);          // 1020 (Сумма расчета в чеке).
			ADD_FISCAL_FIELD(Cashier,                String);       // 1021 (Кассир).
			ADD_FISCAL_FIELD(PayOffSubjectQuantity,  FVLN);         // 1023 (Количество предмета расчета).
			ADD_FISCAL_FIELD(UnitName,               String);       // 1030 (Наименование товара).
			ADD_FISCAL_FIELD(AutomaticNumber,        String);       // 1036 (Номер автомата).
			ADD_FISCAL_FIELD(RNM,                    String);       // 1037 (Регистрационный номер ККТ).
			ADD_FISCAL_FIELD(SessionNumber,          UINT32);       // 1038 (Номер смены).
			ADD_FISCAL_FIELD(FDNumber,               UINT32);       // 1040 (Номер ФД).
			ADD_FISCAL_FIELD(SerialFSNumber,         String);       // 1041 (Заводской номер ФН).
			ADD_FISCAL_FIELD(DocumentNumber,         UINT32);       // 1042 (Номер чека за смену).
			ADD_FISCAL_FIELD(PayOffSubjectAmount,    VLN);          // 1043 (Стоимость предмета расчета).
			ADD_FISCAL_FIELD(OFDName,                String);       // 1046 (Наименование ОФД).
			ADD_FISCAL_FIELD(LegalOwner,             String);       // 1048 (Наименование юр. лица владельца).
			ADD_FISCAL_FIELD(PayOffType,             Byte);         // 1054 (Признак расчета).
			ADD_FISCAL_FIELD(TaxSystem,              Byte);         // 1055 (СНО на платеже).
			ADD_FISCAL_FIELD(EncryptionMode,         Byte);         // 1056 (Признак шифрования).
			ADD_FISCAL_FIELD(AgentFlagsRegistered,   Byte);         // 1057 (Признак(и) платежного агента из итогов регистрации).
			ADD_FISCAL_FIELD(PayOffSubject,          STLV);         // 1059 (Предмет расчета).
			ADD_FISCAL_FIELD(FTSURL,                 String);       // 1060 (Адрес сайта ФНС).
			ADD_FISCAL_FIELD(TaxSystemsRegistered,   Byte);         // 1062 (СНО из итогов регистрации).
			ADD_FISCAL_FIELD(FDSign,                 ByteArray);    // 1077 (Фискальный признак документа).
			ADD_FISCAL_FIELD(PayOffSubjectUnitPrice, VLN);          // 1079 (Цена за единицу предмета расчета с учетом скидок и наценок).

			ADD_FISCAL_FIELD(CashFiscalTotal,         VLN);    // 1031 (Сумма по чеку (БСО) наличными).
			ADD_FISCAL_FIELD(CardFiscalTotal,         VLN);    // 1081 (Сумма по чеку (БСО) электронными).
			ADD_FISCAL_FIELD(PrePaymentFiscalTotal,   VLN);    // 1215 (Сумма по чеку (БСО) предоплатой (зачетом аванса).
			ADD_FISCAL_FIELD(PostPaymentFiscalTotal,  VLN);    // 1216 (Сумма по чеку (БСО) постоплатой (в кредит).
			ADD_FISCAL_FIELD(CounterOfferFiscalTotal, VLN);    // 1217 (Сумма по чеку (БСО) встречным предоставлением).

			ADD_FISCAL_FIELD(TaxAmount02, VLN);    // 1102 (Сумма НДС чека по ставке 18%).
			ADD_FISCAL_FIELD(TaxAmount03, VLN);    // 1103 (Сумма НДС чека по ставке 10%).
			ADD_FISCAL_FIELD(TaxAmount04, VLN);    // 1104 (Сумма расчета по чеку с НДС по ставке 0%).
			ADD_FISCAL_FIELD(TaxAmount05, VLN);    // 1105 (Сумма расчета по чеку без НДС).
			ADD_FISCAL_FIELD(TaxAmount06, VLN);    // 1106 (Сумма НДС чека по расчетной ставке 18/118).
			ADD_FISCAL_FIELD(TaxAmount07, VLN);    // 1107 (Сумма НДС чека по расчетной ставке 10/110).

			ADD_FISCAL_FIELD(InternetMode,       Byte);      // 1108 (Признак работы с интернет (без принтера).
			ADD_FISCAL_FIELD(ServiceAreaMode,    Byte);      // 1109 (Признак применения в сфере услуг).
			ADD_FISCAL_FIELD(FixedReportingMode, Byte);      // 1110 (Признак работы с бланками строгой отчетности (БСО).
			ADD_FISCAL_FIELD(LotteryMode,        Byte);      // 1126 (Признак проведения лотереи).
			ADD_FISCAL_FIELD(PayOffPlace,        String);    // 1187 (Место расчетов).
			ADD_FISCAL_FIELD(GamblingMode,       Byte);      // 1193 (Признак проведения азартных игр).
			ADD_FISCAL_FIELD(VATRate,            Byte);      // 1199 (Ставка НДС).
			ADD_FISCAL_FIELD(CashierINN,         String);    // 1203 (ИНН кассира).
			ADD_FISCAL_FIELD(ExcisableUnitMode,  Byte);      // 1207 (Признак торговли подакцизными товарами).
			ADD_FISCAL_FIELD(OFDURL,             String);    // 1208 (Адрес сайта для получения чека).
			ADD_FISCAL_FIELD(PayOffSubjectType,  Byte);      // 1212 (Признак предмета расчета).
			ADD_FISCAL_FIELD(PayOffMethodType,   Byte);      // 1214 (Признак способа расчета).
			ADD_FISCAL_FIELD(AgentFlag,          Byte);      // 1222 (Признак платежного агента на платеже).
		}
	};

	//---------------------------------------------------------------------------
	// Список полей для получения из ФН после закрытия ФД.
	const QSet<int> FSRequired = QSet<int>()
		<< SDK::Driver::FiscalFields::PayOffSubject
		<< SDK::Driver::FiscalFields::TaxAmount02
		<< SDK::Driver::FiscalFields::TaxAmount03
		<< SDK::Driver::FiscalFields::TaxAmount04
		<< SDK::Driver::FiscalFields::TaxAmount05
		<< SDK::Driver::FiscalFields::TaxAmount06
		<< SDK::Driver::FiscalFields::TaxAmount07
		<< SDK::Driver::FiscalFields::FDDateTime
		<< SDK::Driver::FiscalFields::FDSign
		<< SDK::Driver::FiscalFields::PayOffAmount
		<< SDK::Driver::FiscalFields::CashFiscalTotal
		<< SDK::Driver::FiscalFields::CardFiscalTotal
		<< SDK::Driver::FiscalFields::PrePaymentFiscalTotal
		<< SDK::Driver::FiscalFields::PostPaymentFiscalTotal
		<< SDK::Driver::FiscalFields::CounterOfferFiscalTotal;

	// Список полей итогов для контроля 0-х сумм.
	const QSet<int> FiscalTotals = QSet<int>()
		<< SDK::Driver::FiscalFields::CashFiscalTotal
		<< SDK::Driver::FiscalFields::CardFiscalTotal
		<< SDK::Driver::FiscalFields::PrePaymentFiscalTotal
		<< SDK::Driver::FiscalFields::PostPaymentFiscalTotal
		<< SDK::Driver::FiscalFields::CounterOfferFiscalTotal;

	//---------------------------------------------------------------------------
	class ConfigCleaner
	{
	public:
		ConfigCleaner(QVariantMap * aData = nullptr) : mData(aData) {}

		~ConfigCleaner()
		{
			if (mData)
			{
				using namespace CHardware::FiscalFields;

				QStringList fieldNames = QStringList()
					<< Cashier
					<< CashierINN
					<< UserContact
					<< TaxSystem
					<< AgentFlag;

				foreach (const QString & fieldName, fieldNames)
				{
					mData->remove(fieldName);
				}
			}
		}

	private:
		QVariantMap * mData;
	};

}}    // namespace CFR::FiscalFields

//---------------------------------------------------------------------------
