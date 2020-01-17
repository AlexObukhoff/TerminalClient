/* @file Описатель фискальных реквизитов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDate>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/FR/FiscalFields.h>

// Modules
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/ContainerProxy.h"

// Project
#include "Hardware/FR/FFDataTypes.h"

//---------------------------------------------------------------------------
namespace CFR
{
	/// Сменилась ли ставка НДС с 18% на 20% в РФ.
	inline bool isRFVAT20() { return QDate::currentDate() >= QDate(2019, 1, 1); }
}

//---------------------------------------------------------------------------
namespace CFR { namespace FiscalFields
{
	namespace Types
	{
		#define ADD_FISCAL_TYPE(aType, aMinSize, ...) append(ETypes::aType, SData(aMinSize, #aType, __VA_ARGS__))

		class CData: public CSpecification<ETypes::Enum, SData>
		{
		public:
			CData()
			{
				ADD_FISCAL_TYPE(String,    0);
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
	/// Список фискальных тегов.
	typedef QList<int> TFields;
	typedef ContainerProxy<QList<int>> CFields;

	typedef QMap<int, SData> TData;
	typedef QMap<int, QString> TDescriptionData;
	typedef QPair<TData, TDescriptionData> TAllData;

	//---------------------------------------------------------------------------
	class Data: public CSpecification<int, SData>
	{
	public:
		Data();
		void add(const TData & aData);

		static TAllData process(int aField, const SData & aData = SData());

		int getKey(const QString & aTextKey) const;
		TFields getKeys(const QStringList & aTextKeys) const;
		QStringList getTextKeys() const;
		QStringList getTextKeys(const TFields & aFields) const;
		QString getTextLog(int aField) const;
		QString getLogFromList(const TFields & aFields) const;

	private:
		void checkRFVAT20(int aField);
		QMap<int, QString> mDescriptionData;
	};

	#define ADD_FISCAL_FIELD(aField, aName, aType, ...) const int aName = [] () -> int { CFR::FiscalFields::Data::process(aField, CFR::FiscalFields::SData(CFR::FiscalFields::ETypes::aType, CFiscalSDK::aName, __VA_ARGS__)); return aField; } ();

	ADD_FISCAL_FIELD(1000, FDName,                 String,    QCoreApplication::translate("FiscalFields", "#fd_name"));                                                // 1000 (Наименование фискального документа).
	ADD_FISCAL_FIELD(1008, UserContact,            String,    QCoreApplication::translate("FiscalFields", "#user_contact"));                                           // 1008 (Телефон или электронный адрес покупателя).
	ADD_FISCAL_FIELD(1009, PayOffAddress,          String);                                                                                                            // 1009 (Адрес расчетов).
	ADD_FISCAL_FIELD(1012, FDDateTime,             UnixTime);                                                                                                          // 1012 (Дата и время ФД).
	ADD_FISCAL_FIELD(1013, SerialFRNumber,         String,    QCoreApplication::translate("FiscalFields", "#serial_fr_number"));                                       // 1013 (Заводской номер ФР).
	ADD_FISCAL_FIELD(1017, OFDINN,                 String,    QCoreApplication::translate("FiscalFields", "#ofd_inn"),       CFR::FiscalFields::EClassType::INN);      // 1017 (ИНН ОФД).
	ADD_FISCAL_FIELD(1018, INN,                    String,    QCoreApplication::translate("FiscalFields", "#inn"),           CFR::FiscalFields::EClassType::INN);      // 1018 (ИНН пользователя).
	ADD_FISCAL_FIELD(1020, PayOffAmount,           VLN,       QCoreApplication::translate("FiscalFields", "#payoff_amount"), CFR::FiscalFields::EClassType::Money);    // 1020 (Сумма расчета в чеке).
	ADD_FISCAL_FIELD(1021, Cashier,                String,    QCoreApplication::translate("FiscalFields", "#cashier"));                                                // 1021 (Кассир).
	ADD_FISCAL_FIELD(1030, UnitName,               String);                                                                                                            // 1030 (Наименование товара).
	ADD_FISCAL_FIELD(1036, AutomaticNumber,        String,    QCoreApplication::translate("FiscalFields", "#automatic_number"));                                       // 1036 (Номер автомата).
	ADD_FISCAL_FIELD(1037, RNM,                    String,    QCoreApplication::translate("FiscalFields", "#rnm"));                                                    // 1037 (Регистрационный номер ККТ).
	ADD_FISCAL_FIELD(1038, SessionNumber,          UINT32,    QCoreApplication::translate("FiscalFields", "#session_number"));                                         // 1038 (Номер смены).
	ADD_FISCAL_FIELD(1040, FDNumber,               UINT32,    QCoreApplication::translate("FiscalFields", "#fd_number"));                                              // 1040 (Номер ФД).
	ADD_FISCAL_FIELD(1041, SerialFSNumber,         String,    QCoreApplication::translate("FiscalFields", "#serial_fs_number"));                                       // 1041 (Заводской номер ФН).
	ADD_FISCAL_FIELD(1042, DocumentNumber,         UINT32,    QCoreApplication::translate("FiscalFields", "#document_number"));                                        // 1042 (Номер чека за смену).
	ADD_FISCAL_FIELD(1046, OFDName,                String,    QCoreApplication::translate("FiscalFields", "#ofd_name"));                                               // 1046 (Наименование ОФД).
	ADD_FISCAL_FIELD(1048, LegalOwner,             String);                                                                                                            // 1048 (Наименование юр. лица владельца).
	ADD_FISCAL_FIELD(1054, PayOffType,             Byte);                                                                                                              // 1054 (Признак расчета).
	ADD_FISCAL_FIELD(1055, TaxSystem,              Byte,      QCoreApplication::translate("FiscalFields", "#tax_system"), CFR::FiscalFields::ERequired::Yes);          // 1055 (СНО на платеже).
	ADD_FISCAL_FIELD(1060, FTSURL,                 String,    QCoreApplication::translate("FiscalFields", "#fts_url"));                                                // 1060 (Адрес сайта ФНС).
	ADD_FISCAL_FIELD(1062, TaxSystemsReg,          Byte,      QCoreApplication::translate("FiscalFields", "#tax_systems_reg"));                                        // 1062 (СНО из итогов регистрации).
	ADD_FISCAL_FIELD(1074, ProcessingPhone,        String,    QCoreApplication::translate("FiscalFields", "#processing_phone"), CFR::FiscalFields::ERequired::Yes);    // 1074 (Телефон оператора по приему платежей).
	ADD_FISCAL_FIELD(1077, FDSign,                 ByteArray, QCoreApplication::translate("FiscalFields", "#fd_sign"));                                                // 1077 (Фискальный признак документа).
	ADD_FISCAL_FIELD(1097, OFDNotSentFDQuantity,   UINT32,    QCoreApplication::translate("FiscalFields", "#ofd_not_sent_fd_quantity"));                               // 1097 (Количество непереданных ФД).
	ADD_FISCAL_FIELD(1098, OFDNotSentFDDateTime,   UnixTime,  QCoreApplication::translate("FiscalFields", "#ofd_not_sent_fd_date_time"));                              // 1098 (Дата и время первого из непереданных ФД).
	ADD_FISCAL_FIELD(1101, ReregistrationCause,    Byte,      QCoreApplication::translate("FiscalFields", "#reregistration_cause"));                                   // 1101 (Код причины перерегистрации).
	ADD_FISCAL_FIELD(1111, FDForSessionTotal,      UINT32,    QCoreApplication::translate("FiscalFields", "#fd_for_session_total"));                                   // 1111 (Общее количество ФД за смену).
	ADD_FISCAL_FIELD(1117, SenderMail,             String,    QCoreApplication::translate("FiscalFields", "#sender_mail"));                                            // 1117 (Электронная почта отправителя чека).
	ADD_FISCAL_FIELD(1118, FiscalsForSessionTotal, UINT32,    QCoreApplication::translate("FiscalFields", "#fiscals_for_session_total"));                              // 1118 (Количество кассовых чеков (БСО) за смену).
	ADD_FISCAL_FIELD(1187, PayOffPlace,            String,    QCoreApplication::translate("FiscalFields", "#payoff_place"));                                           // 1187 (Место расчетов).
	ADD_FISCAL_FIELD(1188, ModelVersion,           String,    QCoreApplication::translate("FiscalFields", "#model_version"));                                          // 1188 (Версия модели ККТ).
	ADD_FISCAL_FIELD(1189, FFDFR,                  Byte,      QCoreApplication::translate("FiscalFields", "#ffd_fr"));                                                 // 1189 (Версия ФФД ФР).
	ADD_FISCAL_FIELD(1190, FFDFS,                  Byte,      QCoreApplication::translate("FiscalFields", "#ffd_fs"));                                                 // 1190 (Версия ФФД ФН).
	ADD_FISCAL_FIELD(1199, VATRate,                Byte);                                                                                                              // 1199 (Ставка НДС).
	ADD_FISCAL_FIELD(1203, CashierINN,             String,    QCoreApplication::translate("FiscalFields", "#cashier_inn"), CFR::FiscalFields::EClassType::INN);        // 1203 (ИНН кассира).
	ADD_FISCAL_FIELD(1208, OFDURL,                 String,    QCoreApplication::translate("FiscalFields", "#ofd_url"));                                                // 1208 (Адрес сайта для получения чека).
	ADD_FISCAL_FIELD(1209, FFD,                    Byte);                                                                                                              // 1209 (Версия ФФД).

	// Данные оператора перевода
	ADD_FISCAL_FIELD(1005, TransferOperatorAddress, String,   QCoreApplication::translate("FiscalFields", "#transfer_operator_address"), CFR::FiscalFields::ERequired::Yes);                                        // 1005 (Адрес оператора перевода).
	ADD_FISCAL_FIELD(1016, TransferOperatorINN,     String,   QCoreApplication::translate("FiscalFields", "#transfer_operator_inn"),     CFR::FiscalFields::ERequired::Yes, CFR::FiscalFields::EClassType::INN);    // 1016 (ИНН оператора перевода).
	ADD_FISCAL_FIELD(1026, TransferOperatorName,    String,   QCoreApplication::translate("FiscalFields", "#transfer_operator_name"),    CFR::FiscalFields::ERequired::Yes);                                        // 1026 (Наименование оператора перевода).
	ADD_FISCAL_FIELD(1075, TransferOperatorPhone,   String,   QCoreApplication::translate("FiscalFields", "#transfer_operator_phone"),   CFR::FiscalFields::ERequired::Yes);                                        // 1075 (Телефон оператора перевода).

	// Данные поставщика
	ADD_FISCAL_FIELD(1171, ProviderPhone, String, QCoreApplication::translate("FiscalFields", "#provider_phone"), CFR::FiscalFields::ERequired::Yes);                                        // 1171 (Телефон поставщика).
	ADD_FISCAL_FIELD(1226, ProviderINN,   String, QCoreApplication::translate("FiscalFields", "#provider_inn"),   CFR::FiscalFields::ERequired::Yes, CFR::FiscalFields::EClassType::INN);    // 1226 (ИНН поставщика).

	// Данные платежного агента
	ADD_FISCAL_FIELD(1044, AgentOperation,  String, QCoreApplication::translate("FiscalFields", "#agent_operation"), CFR::FiscalFields::ERequired::Yes);    // 1044 (Операция платежного агента).
	ADD_FISCAL_FIELD(1057, AgentFlagsReg,   Byte, CFR::FiscalFields::ERequired::Yes);                                                                       // 1057 (Признак(и) платежного агента из итогов регистрации и на платеже).
	ADD_FISCAL_FIELD(1073, AgentPhone,      String, QCoreApplication::translate("FiscalFields", "#agent_phone"), CFR::FiscalFields::ERequired::Yes);        // 1073 (Телефон платежного агента).
	ADD_FISCAL_FIELD(1222, AgentFlag,       Byte, CFR::FiscalFields::ERequired::Yes);                                                                       // 1222 (Признак платежного агента на платеже).

	// Статусы
	ADD_FISCAL_FIELD(1050, FSExpiredStatus,    Byte,   QCoreApplication::translate("FiscalFields", "#fs_expired_status"));           // 1050 (Признак исчерпания ресурса ФН).
	ADD_FISCAL_FIELD(1051, FSNeedChangeStatus, Byte,   QCoreApplication::translate("FiscalFields", "#fs_need_change_status"));       // 1051 (Признак необходимости срочной замены ФН).
	ADD_FISCAL_FIELD(1052, FSMemoryEnd,        Byte,   QCoreApplication::translate("FiscalFields", "#fs_memory_end_status"));        // 1052 (Признак заполнения памяти ФН).
	ADD_FISCAL_FIELD(1053, OFDNoConnection,    Byte,   QCoreApplication::translate("FiscalFields", "#ofd_no_connection_status"));    // 1053 (Признак превышения времени ожидания ответа ОФД).

	// Режимы работы
	ADD_FISCAL_FIELD(1001,      AutomaticMode, Byte,   QCoreApplication::translate("FiscalFields", "#automatic_mode"));          // 1001 (Признак автоматического режима).
	ADD_FISCAL_FIELD(1002,     AutonomousMode, Byte,   QCoreApplication::translate("FiscalFields", "#autonomous_mode"));         // 1002 (Признак автономного режима).
	ADD_FISCAL_FIELD(1056,     EncryptionMode, Byte,   QCoreApplication::translate("FiscalFields", "#encryption_mode"));         // 1056 (Признак шифрования).
	ADD_FISCAL_FIELD(1108,       InternetMode, Byte,   QCoreApplication::translate("FiscalFields", "#internet_mode"));           // 1108 (Признак работы с интернет (без принтера).
	ADD_FISCAL_FIELD(1109,    ServiceAreaMode, Byte,   QCoreApplication::translate("FiscalFields", "#service_area_mode"));       // 1109 (Признак применения в сфере услуг).
	ADD_FISCAL_FIELD(1110, FixedReportingMode, Byte,   QCoreApplication::translate("FiscalFields", "#fixed_reporting_mode"));    // 1110 (Признак работы с бланками строгой отчетности (БСО).
	ADD_FISCAL_FIELD(1126,        LotteryMode, Byte,   QCoreApplication::translate("FiscalFields", "#lottery_mode"));            // 1126 (Признак проведения лотереи).
	ADD_FISCAL_FIELD(1193,       GamblingMode, Byte,   QCoreApplication::translate("FiscalFields", "#gambling_mode"));           // 1193 (Признак проведения азартных игр).
	ADD_FISCAL_FIELD(1207,  ExcisableUnitMode, Byte,   QCoreApplication::translate("FiscalFields", "#excisable_unit_mode"));     // 1207 (Признак торговли подакцизными товарами).
	ADD_FISCAL_FIELD(1221,     InAutomateMode, Byte,   QCoreApplication::translate("FiscalFields", "#in_automate_mode"));        // 1221 (Признак установки в автомате).

	// Предмет расчета (на конкретную продажу)
	ADD_FISCAL_FIELD(1023, PayOffSubjectQuantity,   FVLN);                                         // 1023 (Количество предмета расчета).
	ADD_FISCAL_FIELD(1043, PayOffSubjectAmount,     VLN, CFR::FiscalFields::EClassType::Money);    // 1043 (Стоимость предмета расчета).
	ADD_FISCAL_FIELD(1059, PayOffSubject,           STLV);                                         // 1059 (Предмет расчета).
	ADD_FISCAL_FIELD(1079, PayOffSubjectUnitPrice,  VLN, CFR::FiscalFields::EClassType::Money);    // 1079 (Цена за единицу предмета расчета с учетом скидок и наценок).
	ADD_FISCAL_FIELD(1200, PayOffSubjectTaxAmount,  VLN, CFR::FiscalFields::EClassType::Money);    // 1200 (Сумма НДС за предмет расчета).
	ADD_FISCAL_FIELD(1212, PayOffSubjectType,       Byte);                                         // 1212 (Признак предмета расчета).
	ADD_FISCAL_FIELD(1214, PayOffSubjectMethodType, Byte);                                         // 1214 (Признак способа расчета).

	 // Суммы по способу расчета (на весь чек)
	ADD_FISCAL_FIELD(1031,         CashFiscalTotal, VLN, QCoreApplication::translate("FiscalFields", "#cash_fiscal_total"),          CFR::FiscalFields::EClassType::Money);    // 1031 (Сумма по чеку (БСО) наличными).
	ADD_FISCAL_FIELD(1081,         CardFiscalTotal, VLN, QCoreApplication::translate("FiscalFields", "#card_fiscal_total"),          CFR::FiscalFields::EClassType::Money);    // 1081 (Сумма по чеку (БСО) электронными).
	ADD_FISCAL_FIELD(1215,   PrePaymentFiscalTotal, VLN, QCoreApplication::translate("FiscalFields", "#prepayment_fiscal_total"),    CFR::FiscalFields::EClassType::Money);    // 1215 (Сумма по чеку (БСО) предоплатой (зачетом аванса).
	ADD_FISCAL_FIELD(1216,  PostPaymentFiscalTotal, VLN, QCoreApplication::translate("FiscalFields", "#postpayment_fiscal_total"),   CFR::FiscalFields::EClassType::Money);    // 1216 (Сумма по чеку (БСО) постоплатой (в кредит).
	ADD_FISCAL_FIELD(1217, CounterOfferFiscalTotal, VLN, QCoreApplication::translate("FiscalFields", "#counter_offer_fiscal_total"), CFR::FiscalFields::EClassType::Money);    // 1217 (Сумма по чеку (БСО) встречным предоставлением).

	// Налоги (на весь чек)
	ADD_FISCAL_FIELD(1102, TaxAmount02, VLN, QCoreApplication::translate("FiscalFields", "#tax_amount_02"), CFR::FiscalFields::EClassType::Money);    // 1102 (Сумма НДС чека по ставке 18(20)%).
	ADD_FISCAL_FIELD(1103, TaxAmount03, VLN, QCoreApplication::translate("FiscalFields", "#tax_amount_03"), CFR::FiscalFields::EClassType::Money);    // 1103 (Сумма НДС чека по ставке 10%).
	ADD_FISCAL_FIELD(1104, TaxAmount04, VLN, QCoreApplication::translate("FiscalFields", "#tax_amount_04"), CFR::FiscalFields::EClassType::Money);    // 1104 (Сумма расчета по чеку с НДС по ставке 0%).
	ADD_FISCAL_FIELD(1105, TaxAmount05, VLN, QCoreApplication::translate("FiscalFields", "#tax_amount_05"), CFR::FiscalFields::EClassType::Money);    // 1105 (Сумма расчета по чеку без НДС).
	ADD_FISCAL_FIELD(1106, TaxAmount06, VLN, QCoreApplication::translate("FiscalFields", "#tax_amount_06"), CFR::FiscalFields::EClassType::Money);    // 1106 (Сумма НДС чека по расчетной ставке 18/118 (20/120)).
	ADD_FISCAL_FIELD(1107, TaxAmount07, VLN, QCoreApplication::translate("FiscalFields", "#tax_amount_07"), CFR::FiscalFields::EClassType::Money);    // 1107 (Сумма НДС чека по расчетной ставке 10/110).

	//---------------------------------------------------------------------------
	// Список полей итогов для контроля 0-х сумм.
	const TFields FiscalTotals = TFields()
		<< CFR::FiscalFields::CashFiscalTotal
		<< CFR::FiscalFields::CardFiscalTotal
		<< CFR::FiscalFields::PrePaymentFiscalTotal
		<< CFR::FiscalFields::PostPaymentFiscalTotal
		<< CFR::FiscalFields::CounterOfferFiscalTotal;

	//---------------------------------------------------------------------------
	// Список полей предмета расчета.
	const TFields PayOffSubjectFields = TFields()
		<< CFR::FiscalFields::UnitName
		<< CFR::FiscalFields::VATRate
		<< CFR::FiscalFields::PayOffSubjectQuantity
		<< CFR::FiscalFields::PayOffSubjectAmount
		<< CFR::FiscalFields::PayOffSubjectUnitPrice
		<< CFR::FiscalFields::PayOffSubjectTaxAmount
		<< CFR::FiscalFields::PayOffSubjectType
		<< CFR::FiscalFields::PayOffSubjectMethodType;

	//---------------------------------------------------------------------------
	// Список полей налогов.
	class TaxAmountFields: public CFields
	{
	public:
		TaxAmountFields(): CFields(CFields()
			<< CFR::FiscalFields::TaxAmount02
			<< CFR::FiscalFields::TaxAmount03
			<< CFR::FiscalFields::TaxAmount04
			<< CFR::FiscalFields::TaxAmount05
			<< CFR::FiscalFields::TaxAmount06
			<< CFR::FiscalFields::TaxAmount07) {}
	};

	//---------------------------------------------------------------------------
	// Список полей, которые необходимо ставить на платеже, если есть флаг агента.
	const TFields AgentFlagAssociatedFields = TFields()
		<< CFR::FiscalFields::ProviderPhone
		<< CFR::FiscalFields::ProcessingPhone
		<< CFR::FiscalFields::AgentPhone
		<< CFR::FiscalFields::AgentOperation

		<< CFR::FiscalFields::TransferOperatorAddress
		<< CFR::FiscalFields::TransferOperatorINN
		<< CFR::FiscalFields::TransferOperatorName
		<< CFR::FiscalFields::TransferOperatorPhone;

	//---------------------------------------------------------------------------
	// Список полей, которые обновляются для каждого фискального чека.
	const TFields RenewableFields = AgentFlagAssociatedFields + TFields()
		<< CFR::FiscalFields::Cashier
		<< CFR::FiscalFields::CashierINN
		<< CFR::FiscalFields::UserContact
		<< CFR::FiscalFields::SenderMail
		<< CFR::FiscalFields::TaxSystem
		<< CFR::FiscalFields::AgentFlag;

	//---------------------------------------------------------------------------
	// Режимы работы
	const TFields ModeFields = TFields()
		<< CFR::FiscalFields::AutomaticMode
		<< CFR::FiscalFields::AutonomousMode
		<< CFR::FiscalFields::EncryptionMode
		<< CFR::FiscalFields::InternetMode
		<< CFR::FiscalFields::ServiceAreaMode
		<< CFR::FiscalFields::FixedReportingMode
		<< CFR::FiscalFields::LotteryMode
		<< CFR::FiscalFields::GamblingMode
		<< CFR::FiscalFields::ExcisableUnitMode
		<< CFR::FiscalFields::InAutomateMode;

}}    // namespace CFR::FiscalFields

//---------------------------------------------------------------------------
