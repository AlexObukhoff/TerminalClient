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

	/// Получить список полей для чека баланса.
	QVariantMap getFields() const
	{
		QVariantMap fields;

		auto fillFields = [&](const QString & aPrefix, const QList<SAmounts> & aSums)
		{
			fields[aPrefix + "BILL_COUNT"] = 0;
			fields[aPrefix + "BILL_SUM"] = 0;
			fields[aPrefix + "COIN_COUNT"] = 0;
			fields[aPrefix + "COIN_SUM"] = 0;

			fields[aPrefix + "EMONEY_SUM"] = 0;

			QMap<EAmountType::Enum, double> sumByType;

			if (isValid)
			{
				fields["ENCASHMENT_START_DATE"] = lastEncashmentDate.toLocalTime().toString("dd.MM.yyyy hh:mm:ss");
				fields[aPrefix + "TOTAL_SUM"] = amount;


				foreach (const SAmounts & sum, aSums)
				{
					QString type;

					switch (sum.type)
					{
						case EAmountType::Bill: type = "BILL"; break;
						case EAmountType::Coin: type = "COIN"; break;
						case EAmountType::EMoney: type = "EMONEY"; break;
						default: type = "BILL";
					}

					foreach (auto amount, sum.amounts)
					{
						fields[aPrefix + amount.value.toString() + "_" + type + "_COUNT"] = amount.count;
						fields[aPrefix + amount.value.toString() + "_" + type + "_SUM"] = amount.value.toDouble() * amount.count;

						// Увеличиваем общее количество купюр одного типа.
						fields[aPrefix + type + "_COUNT"] = fields[aPrefix + type + "_COUNT"].toInt() + amount.count;

						// Добавляем детализацию по купюрам (номера купюр каждого номинала).
						fields[aPrefix + amount.value.toString() + "_" + type + "_DETAILS"] = amount.serials;

						if (sumByType.contains(sum.type))
						{
							sumByType[sum.type] += amount.value.toDouble() * amount.count;
						}
						else
						{
							sumByType[sum.type] = amount.value.toDouble() * amount.count;
						}
					}
				}

				fields[aPrefix + "BILL_SUM"] = QString::number(sumByType[EAmountType::Bill], 'f', 2);
				fields[aPrefix + "COIN_SUM"] = QString::number(sumByType[EAmountType::Coin], 'f', 2);
				fields[aPrefix + "EMONEY_SUM"] = QString::number(sumByType[EAmountType::EMoney], 'f', 2);
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
