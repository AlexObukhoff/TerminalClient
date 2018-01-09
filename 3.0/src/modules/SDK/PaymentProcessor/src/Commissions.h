/* @file Средства подсчёта комиссий. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QTime>
#include <QtCore/QSet>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// Common
#include <Common/PropertyTree.h>

namespace SDK {
namespace PaymentProcessor {

//----------------------------------------------------------------------------
namespace CCommissions
{
	const double DefaultAboveValue = 0.0;
	const double DefaultBelowValue = 1000000.0;
	const double MinChargeValue = 0.0;
	const double MaxChargeValue = 1000000.0;
}

//----------------------------------------------------------------------------
class Commission
{
	friend class CommissionList;
	friend class Commissions;

public:
	enum Type
	{
		Absolute,
		Percent
	};

	enum RoundType
	{
		High,
		Bank,
		Low
	};

	enum Day
	{
		Mon = 1,
		Tue,
		Wed,
		Thu,
		Fri,
		Sat,
		Sun
	};

	enum Base
	{
		AmountAll,
		Amount
	};

	Commission();

	/// Получение левой границы действия комиссии.
	double getMinLimit() const;

	/// Получение правой границы действия комиссии.
	double getMaxLimit() const;

	/// Если для комиссии определен нижний лимит, то предикат возвращает true.
	bool hasMinLimit() const;

	/// Если для комиссии определен верхний лимит, то предикат возвращает true.
	bool hasMaxLimit() const;

	/// Если для комиссии определены лимиты, то предикат возвращает true.
	bool hasLimits() const;

	/// Возвращает комиссию для суммы aSum.
	double getValueFor(double aSum, bool aAtAmount) const;

	/// Возвращает размер комиссии в процентах или денежных единицах в зависимости от типа.
	double getValue() const;

	/// Возвращает минимальный размер комиссии.
	double getMinCharge() const;

	/// Возвращает максимальный размер комиссии.
	double getMaxCharge() const;

	/// Возвращает тип комиссии.
	Type getType() const;

	/// Возвращает базу рассчёта комиссии
	Base getBase() const;

protected:
	bool contains(double aSum) const;

	bool operator >(const Commission & aOther) const;
	bool operator <(const Commission & aOther) const;

	/// Конструирование готового объекта по переданным настройкам.
	static Commission fromSettings(const TPtree & aSettings);

private:
	double mValue;
	double mAbove;
	double mBelow;
	double mMinCharge;
	double mMaxCharge;
	Type mType;
	RoundType mRound;
	Base mBase;
};

//----------------------------------------------------------------------------
typedef QList<Commission> TCommissions;

//----------------------------------------------------------------------------
class CommissionList
{
	friend class CommissionByTimeList;
	friend class CommissionByDayList;
	friend class Commissions;

protected:
	CommissionList();

	TCommissions getCommissions() const;

	bool query(double aSum, Commission & aCommission) const;

	CommissionList & operator <<(const Commission & aCommission);

	static CommissionList fromSettings(const TPtree & aSettings);

private:
	TCommissions mCommissions;
};

//----------------------------------------------------------------------------
class CommissionByTimeList
{
	friend class CommissionByDayList;
	friend class Commissions;

protected:
	CommissionByTimeList();

	TCommissions getCommissions() const;

	bool query(double aSum, Commission & aCommission) const;

	CommissionByTimeList & operator <<(const Commission & aCommission);

	static CommissionByTimeList fromSettings(const TPtree & aSettings);

private:
	QTime mBegin;
	QTime mEnd;

	CommissionList mCommissions;
};

//----------------------------------------------------------------------------
class CommissionByDayList
{
	friend class Commissions;

protected:
	CommissionByDayList();

	TCommissions getCommissions() const;

	bool query(double aSum, Commission & aCommission) const;

	static CommissionByDayList fromSettings(const TPtree & aSettings);

private:
	QSet<Commission::Day> mDays;

	QList<CommissionByTimeList> mCommissionsByTime;
	CommissionList mCommissions;
};

//----------------------------------------------------------------------------
class ProcessingCommission
{
	enum Type
	{
		// <Комиссия> = <amount>*<процент комисии киберплат>
		Real = 1,
		// Если <amount_all> меньше <amount>, то <Комиссия>=0
		// Иначе <Комиссия> = (<amount_all> - <amount>)*<процент комисии киберплат >
		Diff,
		// <Комиссия> = <amount>*<процент комисии киберплат>/ (1 - <процент комисии киберплат>)
		Inverse
	};

public:
	ProcessingCommission();

	/// Производит рассчёт комиссии в зависимости от сумм платежа.
	double getValue(double aAmount, double aAmountAll);

	static ProcessingCommission fromSettings(const TPtree & aSettings);

	/// Проверка на пустую комиссию
	bool isNull() const;

private:
	Type mType;
	double mValue;
	double mMinValue;
};

//----------------------------------------------------------------------------
class Commissions
{
	struct SComplexCommissions
	{
		static bool sortByMinLimit(const Commission & aFirst, const Commission & aSecond);

		TCommissions getCommissions() const;

		Commission query(double aSum) const;

		QList<CommissionByDayList> commissionsByDay;
		QList<CommissionByTimeList> commissionsByTime;
		CommissionList comissions;
		int vat;
	};

public:
	Commissions();

	/// Получение списка актуальных комиссий по оператору.
	TCommissions getCommissions(qint64 aOperator) const;

	/// Получение актуальной комиссии по идентификатору оператора и сумме платежа.
	Commission getCommission(qint64 aProvider, double aSum) const;

	/// Получение комиссии процессинга за платёж по указанному оператору. Используется для разбивки комиссии
	/// на платёжном чеке. Все подсчёты производятся только методом getCommission.
	ProcessingCommission getProcessingCommission(qint64 aProvider);

	/// Получение размера НДС для тела платежа указанного провайдера
	int getVAT(qint64 aProvider);

	/// Если во время загрузки комиссий произошла ошибка, то этот метод вернёт false.
	bool isValid() const;

	/// Чтение комиссий из настроек.
	static Commissions fromSettings(const TPtree & aSettings);

	/// Дополнить комиссии недостающими элементами из настроек
	void appendFromSettings(const TPtree & aSettings);

protected:
	SComplexCommissions loadCommissions(const TPtree & aBranch);

private:
	bool mIsValid;
	QMap<qint64, SComplexCommissions> mProviderCommissions;
	QMap<qint64, ProcessingCommission> mProcessingCommissions;
	SComplexCommissions mDefaultCommissions;
};

//----------------------------------------------------------------------------
}} // SDK::PaymentProcessor

//----------------------------------------------------------------------------
