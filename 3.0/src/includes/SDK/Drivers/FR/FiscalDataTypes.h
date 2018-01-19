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
namespace ETaxations
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
typedef QMap<ETaxations::Enum, QString> TTaxationData;

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
		PostPayment,    /// Аванс
		Credit,         /// Кредит
		CounterOffer    /// Встречное предоставление
	};
}

//--------------------------------------------------------------------------------
/// Признак способа расчета (1214).
namespace EPayOffMethodTypes
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
		Credit             /// Оплата кредита
	};
}

//--------------------------------------------------------------------------------
/// Признак предмета расчета (1212).
namespace EPayOffSubjectTypes
{
	enum Enum
	{
		None = 0,        /// Отсутствует
		Payment = 10,    /// Платеж
		AgentFee         /// Агентское вознаграждение
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

struct SAmountData
{
	TSum sum;        /// Сумма платежа.
	TVAT VAT;        /// НДС (value added tax).
	QString name;    /// Локализованное название платежа (товар).
	EPayOffSubjectTypes::Enum payOffSubjectType;    /// Признак предмета расчета.
	int section;     /// Отдел.

	SAmountData() : sum(0), VAT(0), section(-1) {}
	SAmountData(double aSum, TVAT aVAT, const QString & aName, EPayOffSubjectTypes::Enum aPayOffSubjectType, int aSection = -1):
		sum(aSum), VAT(aVAT), name(aName), payOffSubjectType(aPayOffSubjectType), section(aSection) {}
};

typedef QList<SAmountData> TAmountDataList;

struct SPaymentData
{
	TAmountDataList amountDataList;    /// Список данных товара
	bool back;                         /// Признак возврата товара
	EPayTypes::Enum payType;           /// Тип оплаты
	ETaxations::Enum taxation;         /// Система налогообложения (СНО)
	EAgentFlags::Enum agentFlag;       /// Флаг агента
	QVariantMap fiscalParameters;      /// Параметры для фискальной печати (см. CHardware::FiscalFields)

	SPaymentData(): back(false), payType(EPayTypes::None), taxation(ETaxations::None), agentFlag(EAgentFlags::None) {}
	SPaymentData(const TAmountDataList & aAmountDataList, bool aBack, EPayTypes::Enum aPayType = EPayTypes::None, ETaxations::Enum aTaxation = ETaxations::None, EAgentFlags::Enum aAgentFlag = EAgentFlags::None):
		back(aBack), amountDataList(aAmountDataList), taxation(aTaxation), payType(aPayType), agentFlag(aAgentFlag) {}
};

//--------------------------------------------------------------------------------
/// Параметры платежа
typedef QMap<int, QVariant> TFiscalPaymentData;

/// Параметры сумм платежа
typedef QList<TFiscalPaymentData> TComplexFiscalPaymentData;

/// Имена секций
typedef QMap<int, QString> TSectionNames;

}} // namespace SDK::Driver

Q_DECLARE_METATYPE(SDK::Driver::TTaxationData);
Q_DECLARE_METATYPE(SDK::Driver::TAgentFlagsData);
Q_DECLARE_METATYPE(SDK::Driver::TSectionNames);

//--------------------------------------------------------------------------------
