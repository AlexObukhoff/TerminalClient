/* @file Типы данных для фискальных операций (продажи, налоги и пр.) для фискальных регистраторов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSet>
#include <QtCore/QVariantMap>
#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
/// Состояние сессии.
namespace ESessionState
{
	enum Enum
	{
		Error,     /// Ошибка определения.
		Opened,    /// Открыта.
		Closed,    /// Закрыта.
		Expired    /// Истекла.
	};
}

//--------------------------------------------------------------------------------
// Структура описателя фискальных тегов.
struct SFiscalFieldData
{
	QString translationPF;
	bool isMoney;

	SFiscalFieldData(): isMoney(false) {}
	SFiscalFieldData(const QString & aTranslationPF, bool aIsMoney): translationPF(aTranslationPF), isMoney(aIsMoney) {}
};

typedef QMap<QString, SFiscalFieldData> TFiscalFieldData;

//--------------------------------------------------------------------------------
namespace EFiscalAmount
{
	enum Enum
	{
		Payment,
		Commission,
		ProcessingCommission
	};
}

//--------------------------------------------------------------------------------
/// Системы налогообложения - СНО (1062 в ФР, 1055 в чеке).
namespace ETaxSystems
{
	enum Enum
	{
		None                         = 0x00,    /// Отсутствует
		Main                         = 0x01,    /// Общая
		SimplifiedIncome             = 0x02,    /// Упрощенная доход
		SimplifiedIncomeMinusExpense = 0x04,    /// Упрощенная доход минус расход
		SingleImputedIncome          = 0x08,    /// Единый налог на вмененный доход (ЕНВД)
		SingleAgricultural           = 0x10,    /// Единый сельскохозяйственный налог
		Patent                       = 0x20     /// Патентная система налогообложения}
	};
}

/// Описатель списка СНО
typedef QMap<ETaxSystems::Enum, QString> TTaxSystemData;

//--------------------------------------------------------------------------------
/// Признаки агента (1057 в ФР, 1222 в чеке).
namespace EAgentFlags
{
	enum Enum
	{
		None            = 0x00,    /// Отсутствует
		BankAgent       = 0x01,    /// Банковский агент
		BankSubagent    = 0x02,    /// Банковский субагент
		PaymentAgent    = 0x04,    /// Платежный агент
		PaymentSubagent = 0x08,    /// Платежный субагент
		Attorney        = 0x10,    /// Поверенный
		CommissionAgent = 0x20,    /// Комиссионер
		Agent           = 0x40     /// Агент
	};
}

/// Описатель списка признаков агента
typedef QMap<EAgentFlags::Enum, QString> TAgentFlagsData;

//--------------------------------------------------------------------------------
/// Средства оплаты.
namespace EPayTypes
{
	enum Enum
	{
		None,           /// Отсутствует
		Cash,           /// Наличными
		EMoney,         /// Электронными
		PrePayment,     /// Аванс
		PostPayment,    /// Кредит
		CounterOffer    /// Встречное предоставление
	};
}

//--------------------------------------------------------------------------------
/// Признак способа расчета (1214).
namespace EPayOffSubjectMethodTypes
{
	enum Enum
	{
		None = 0,          /// Отсутствует
		Prepayment100,     /// Предоплата 100%
		Prepayment,        /// Предоплата
		PostPayment,       /// Аванс
		Full,              /// Полный расчет
		Part,              /// Частичный расчет и кредит
		CreditTransfer,    /// Передача в кредит
		CreditPayment      /// Оплата кредита
	};
}

//--------------------------------------------------------------------------------
/// Признак предмета расчета (1212).
namespace EPayOffSubjectTypes
{
	enum Enum
	{
		None = 0,                 /// Отсутствует
		Unit,                     /// Товар
		ExciseUnit,               /// Подакцизный товар
		Job,                      /// Работа
		Service,                  /// Услуга
		GamblingBet,              /// Ставка азартной игры
		GamblingWin,              /// Выигрыш азартной игры
		LotteryTicket,            /// Лотерейный билет
		LotteryWin,               /// Выигрыш лотереи
		RIARightsProvision,       /// Предоставление прав на использование результатов интеллектуальной деятельности (РИД, RIA)
		Payment,                  /// Платеж
		AgentFee,                 /// Агентское вознаграждение
		Composite,                /// Составной
		Other,                    /// Иной
		PropertyRight,            /// Имущественное право
		NonSalesIncome,           /// Внереализационный доход
		InsuranceContribution,    /// Страховой взнос
		TradeTax,                 /// Торговый сбор
		ResortTax,                /// Курортный сбор
		Deposit                   /// Залог
	};
}

//--------------------------------------------------------------------------------
/// Признак расчета (1054).
namespace EPayOffTypes
{
	enum Enum
	{
		None = 0,     /// Отсутствует
		Debit,        /// Приход
		DebitBack,    /// Возврат прихода
		Credit,       /// Расход
		CreditBack    /// Возврат расхода
	};
}

//--------------------------------------------------------------------------------
/// Режим работы.
namespace EOperationModes
{
	enum Enum
	{
		None = 0x00,    /// Отсутствует
		Encryption     = 0x01,    /// Шифрование
		Autonomous     = 0x02,    /// Автономный режим
		Automatic      = 0x04,    /// Автоматический режим
		ServiceArea    = 0x08,    /// Применение в сфере услуг
		FixedReporting = 0x10,    /// Бланк строгой отчетности (БСО)
		Internet       = 0x20     /// Применение в интернете
	};
}

/// Описатель списка режимов работы
typedef QMap<EOperationModes::Enum, QString> TOperationModeData;

//--------------------------------------------------------------------------------
/// Налоги
typedef int TVAT;
typedef QSet<TVAT> TVATs;
typedef double TSum;


/*   !!!!!!!!!!!!!!  НЕ забываем обновить метод FiscalProtocol::createRequest и parseRequestJsonDoc */

struct SUnitData
{
	TSum sum;                                                   /// Сумма платежа.
	TVAT VAT;                                                   /// НДС (value added tax).
	QString name;                                               /// Локализованное название платежа (1030, Билайн).
	QString providerName;                                       /// Наименование поставщика товара (1225, ПАО Вымпелком).
	QString providerINN;                                        /// ИНН поставщика товара (1226, оператор/дилер/Платина).
	EPayOffSubjectTypes::Enum payOffSubjectType;                /// Признак предмета расчета (1212).
	EPayOffSubjectMethodTypes::Enum payOffSubjectMethodType;    /// Признак способа расчета (1214).
	int section;                                                /// Отдел.

	SUnitData() : sum(0), VAT(0), payOffSubjectType(EPayOffSubjectTypes::None), payOffSubjectMethodType(EPayOffSubjectMethodTypes::None), section(-1) {}
	SUnitData(
		double aSum,
		TVAT aVAT,
		const QString & aName,
		const QString & aProviderName,
		const QString & aProviderINN,
		EPayOffSubjectTypes::Enum aPayOffSubjectType,
		int aSection = -1,
		EPayOffSubjectMethodTypes::Enum aPayOffSubjectMethodType = EPayOffSubjectMethodTypes::Full):
			sum(aSum),
			VAT(aVAT),
			name(aName),
			providerName(aProviderName),
			providerINN(aProviderINN),
			payOffSubjectType(aPayOffSubjectType),
			payOffSubjectMethodType(aPayOffSubjectMethodType),
			section(aSection) {}
};

typedef QList<SUnitData> TUnitDataList;


/// Фискальные данные платежа  !!!!!!!!!!!!!!  НЕ забываем обновить метод FiscalProtocol::createRequest и parseRequestJsonDoc
struct SPaymentData
{
	TUnitDataList unitDataList;       /// Список данных товара
	EPayOffTypes::Enum payOffType;    /// Признак расчета
	EPayTypes::Enum payType;          /// Тип оплаты
	ETaxSystems::Enum taxSystem;      /// Система налогообложения (СНО)
	EAgentFlags::Enum agentFlag;      /// Флаг агента
	QVariantMap fiscalParameters;     /// Параметры платежа - теги или имеют к ним отношение

	SPaymentData(): payOffType(EPayOffTypes::None), payType(EPayTypes::None), taxSystem(ETaxSystems::None), agentFlag(EAgentFlags::None) {}
	SPaymentData(const TUnitDataList & aUnitDataList, EPayOffTypes::Enum aPayOffType, EPayTypes::Enum aPayType = EPayTypes::None, ETaxSystems::Enum aTaxSystem = ETaxSystems::None, EAgentFlags::Enum aAgentFlag = EAgentFlags::None):
		payOffType(aPayOffType), unitDataList(aUnitDataList), taxSystem(aTaxSystem), payType(aPayType), agentFlag(aAgentFlag) {}

	bool back() const { return (payOffType == EPayOffTypes::DebitBack) || (payOffType == EPayOffTypes::Credit); }
};

//--------------------------------------------------------------------------------
/// Параметры платежа
typedef QVariantMap TFiscalPaymentData;

/// Параметры сумм платежа
typedef QList<TFiscalPaymentData> TComplexFiscalPaymentData;

/// Имена секций
typedef QMap<int, QString> TSectionNames;

}} // namespace SDK::Driver

Q_DECLARE_METATYPE(SDK::Driver::TTaxSystemData);
Q_DECLARE_METATYPE(SDK::Driver::TAgentFlagsData);
Q_DECLARE_METATYPE(QList<SDK::Driver::EAgentFlags::Enum>);
Q_DECLARE_METATYPE(SDK::Driver::TSectionNames);
Q_DECLARE_METATYPE(SDK::Driver::TFiscalFieldData);

//--------------------------------------------------------------------------------
