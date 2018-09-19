/* @file Структуры с описанием данных инкассации. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

#include <SDK/PaymentProcessor/Payment/Amount.h>
#include <Common/Currency.h>

namespace SDK { 
namespace PaymentProcessor {

//---------------------------------------------------------------------------
/// Краткая информация о суммах и платежах с момента последней инкассации.
struct SBalance
{
	/// Описание подробной разбивки по суммам.
	struct SAmounts
	{
		struct SAmount
		{
			Currency::Nominal value;
			int count;
			QString serials;

			SAmount(double aValue, int aCount, const QString & aSerials)
				: value(aValue), count(aCount), serials(aSerials)
			{}
		};

		SAmounts()
		{
			type = EAmountType::Bill;
			currency = 0;
		}

		EAmountType::Enum type; /// Тип сумм (купюры, монеты, оплата по карте и т.п.).
		int currency;           /// Валюта, пока не используется.
		QList<SAmount> amounts; /// Список сумма:количество.

		Currency::Nominal getSum() const
		{
			Currency::Nominal::RawType result = 0;

			foreach (auto & amount, amounts)
			{
				result += amount.value.rawValue() * amount.count;
			}

			return Currency::Nominal::fromRawValue(result);
		}
	};

	SBalance()
	{
		isValid = false;
		lastEncashmentId = 0;
		amount = fee = processed = QString::number(0);
	}

	bool isEmpty() const
	{
		return qFuzzyIsNull(amount.toDouble()) && payments.isEmpty();
	}

	bool isValid;

	int lastEncashmentId;         /// Номер последней инкассации
	QDateTime lastEncashmentDate; /// Дата последней инкассации (если инкассаций не было, нулевая)

	QList<SAmounts> detailedSums; /// Подробная разбивка по суммам
	QList<SAmounts> dispensedSums;/// Подробная разбивка по суммам
	QString amount;               /// Сумма средств в терминале
	QString dispensedAmount;      /// Сумма средств выданных в качестве сдачи
	QString fee;                  /// Сумма комиссий
	QString processed;            /// Сумма проведёных платежей

	QList<qint64> payments;       /// Платежи, входящие в инкассацию
	QSet<qint64> notPrintedPayments; /// Платежи, входящие в инкассацию, но имеющие не напечатанные чеки

	QString dispensedNotes;       /// Список выданных купюр в виде отчета

	QString getAmountSum(const EAmountType::Enum aType, const QList<SAmounts> & aSumsList) const
	{
		Currency::Nominal::RawType result = 0;

		foreach (auto & bal, aSumsList)
		{
			if (bal.type == aType)
			{
				result += bal.getSum().rawValue();
			}
		}

		return Currency::Nominal::fromRawValue(result).toString(false);
	}

	/// Получить список полей для чека баланса.
	QVariantMap getFields() const
	{
		QVariantMap fields;

		auto typeToString = [](EAmountType::Enum aType) -> QString {
			switch (aType)
			{
			case EAmountType::Coin:     return "COIN";
			case EAmountType::EMoney:   return "EMONEY";
			case EAmountType::BankCard: return "BANKCARD";
			default: return "BILL";
			}
		};

		auto fillFields = [&](const QString & aPrefix, const QList<SAmounts> & aSums)
		{
			fields[aPrefix + "BILL_COUNT"] = 0;
			fields[aPrefix + "BILL_SUM"] = 0;
			fields[aPrefix + "COIN_COUNT"] = 0;
			fields[aPrefix + "COIN_SUM"] = 0;

			fields[aPrefix + "EMONEY_SUM"] = 0;
			fields[aPrefix + "BANKCARD_SUM"] = 0;

			if (isValid)
			{
				fields["ENCASHMENT_START_DATE"] = lastEncashmentDate.toLocalTime().toString("dd.MM.yyyy hh:mm:ss");
				fields[aPrefix + "TOTAL_SUM"] = amount;

				foreach (const SAmounts & sum, aSums)
				{
					QString type = typeToString(sum.type);

					foreach (auto a, sum.amounts)
					{
						fields[aPrefix + a.value.toString() + "_" + type + "_COUNT"] = a.count;
						fields[aPrefix + a.value.toString() + "_" + type + "_SUM"] = a.value.toDouble() * a.count;

						// Увеличиваем общее количество купюр одного типа.
						fields[aPrefix + type + "_COUNT"] = fields[aPrefix + type + "_COUNT"].toInt() + a.count;

						// Добавляем детализацию по купюрам (номера купюр каждого номинала).
						fields[aPrefix + a.value.toString() + "_" + type + "_DETAILS"] = a.serials;
					}
				}

				fields[aPrefix + "BILL_SUM"] = getAmountSum(EAmountType::Bill, aSums);
				fields[aPrefix + "COIN_SUM"] = getAmountSum(EAmountType::Coin, aSums);
				fields[aPrefix + "EMONEY_SUM"] = getAmountSum(EAmountType::EMoney, aSums);
				fields[aPrefix + "BANKCARD_SUM"] = getAmountSum(EAmountType::BankCard, aSums);
			}
		};

		fillFields("", detailedSums);
		fillFields("DISPENSED_", dispensedSums);

		fields["TOTAL_SUM"] = amount;
		fields["DISPENSED_TOTAL_SUM"] = dispensedAmount;

		return fields;
	}
};

//---------------------------------------------------------------------------
/// Описание инкассационных данных.
struct SEncashment
{
	SEncashment()
	{
		id = -1;
	}

	bool isValid() const
	{
		return id != -1;
	}

	int id;           /// Номер инкассации
	QDateTime date;   /// Период инкассации по дата/время
	SBalance balance; /// Информация о суммах и платежах в терминале на момент инкассации
	QString report;   /// Отчёт по платежам
	QVariantMap parameters; /// Параметры

	/// Получить список полей для чека инкассации.
	QVariantMap getFields() const
	{
		auto fields = parameters;
		
		fields.unite(balance.getFields());

		fields["ENCASHMENT_END_DATE"] = date.toString("dd.MM.yyyy hh:mm:ss");
		fields["ENCASHMENT_NUMBER"] = id;

		return fields;
	}
};

//---------------------------------------------------------------------------
}} // SDK::PaymentProcessor
