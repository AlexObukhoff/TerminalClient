/* @file Средства подсчёта комиссий. */

// Stl
#include <stdexcept>
#include <limits>
#include <cmath>

// Boost
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Project
#include "Commissions.h"

namespace SDK {
namespace PaymentProcessor {

//----------------------------------------------------------------------------
Commission::Commission()
	: mValue(0.0),
	mAbove(CCommissions::DefaultAboveValue),
	mBelow(CCommissions::DefaultBelowValue),
	mMinCharge(CCommissions::MinChargeValue),
	mMaxCharge(CCommissions::MaxChargeValue),
	mType(Absolute),
	mRound(Bank),
	mBase(AmountAll)
{
}

//----------------------------------------------------------------------------
bool Commission::contains(double aSum) const
{
	return qFuzzyCompare(aSum, mBelow) || ((aSum > mAbove) && (aSum < mBelow));
}

//----------------------------------------------------------------------------
double Commission::getMinLimit() const
{
	return mAbove;
}

//----------------------------------------------------------------------------
double Commission::getMaxLimit() const
{
	return mBelow;
}

//----------------------------------------------------------------------------
bool Commission::hasMinLimit() const
{
	return !qFuzzyCompare(getMinLimit(), CCommissions::DefaultAboveValue);
}

//----------------------------------------------------------------------------
bool Commission::hasMaxLimit() const
{
	return !qFuzzyCompare(getMaxLimit(), CCommissions::DefaultBelowValue);
}

//----------------------------------------------------------------------------
bool Commission::hasLimits() const
{
	return hasMinLimit() || hasMaxLimit();
}

//----------------------------------------------------------------------------
double Commission::getValueFor(double aSum, bool aAtAmount) const
{
	double comission = 0;

	if (mType == Percent)
	{
		if (aAtAmount || mBase == Amount)
		{
			// Комиссия считается по сумме к зачислению
			comission = aSum - aSum / (1 + mValue / 100.0);
		}
		else
		{
			// Комиссия считается по внесённым средствам
			comission = mValue * aSum * 0.01;
		}
	}
	else
	{
		comission = mValue;
	}

	return comission < mMinCharge ? mMinCharge :
		comission > mMaxCharge ? mMaxCharge : comission;
}

//----------------------------------------------------------------------------
double Commission::getValue() const
{
	return mValue;
}

//----------------------------------------------------------------------------
Commission::Type Commission::getType() const
{
	return mType;
}

//----------------------------------------------------------------------------
Commission::Base Commission::getBase() const
{
	return mBase;
}

//----------------------------------------------------------------------------
double Commission::getMinCharge() const
{
	return mMinCharge;
}

//----------------------------------------------------------------------------
double Commission::getMaxCharge() const
{
	return mMaxCharge;
}

//----------------------------------------------------------------------------
bool Commission::operator >(const Commission & aOther) const
{
	if ((qFuzzyCompare(mAbove, aOther.mAbove) && (mBelow < aOther.mBelow)) ||
		(qFuzzyCompare(mBelow, aOther.mBelow) && (mAbove > aOther.mAbove)))
	{
		return true;
	}

	return ((mAbove > aOther.mAbove) && (mBelow < aOther.mBelow));
}

//----------------------------------------------------------------------------
bool Commission::operator <(const Commission & aOther) const
{
	if (qFuzzyCompare(aOther.mBelow, CCommissions::DefaultBelowValue) && qFuzzyCompare(aOther.mAbove, CCommissions::DefaultAboveValue))
	{
		return true;
	}

	if (qFuzzyCompare(mBelow, CCommissions::DefaultBelowValue) && qFuzzyCompare(mAbove, CCommissions::DefaultAboveValue))
	{
		return false;
	}

	return (mAbove < aOther.mAbove);
}

//----------------------------------------------------------------------------
Commission Commission::fromSettings(const TPtree & aSettings)
{
	Commission commission;

	commission.mType = (aSettings.get<QString>("<xmlattr>.type").toLower() == "absolute") ?  Absolute : Percent;
	commission.mValue = aSettings.get<double>("<xmlattr>.amount");

	QString minChargeValue = aSettings.get<QString>("<xmlattr>.min_charge", QString::number(CCommissions::MinChargeValue)).trimmed();
	commission.mMinCharge = minChargeValue.isEmpty() ? CCommissions::MinChargeValue : minChargeValue.toDouble();

	QString maxChargeValue = aSettings.get<QString>("<xmlattr>.max_charge", QString::number(CCommissions::MaxChargeValue)).trimmed();
	commission.mMaxCharge = maxChargeValue.isEmpty() ? CCommissions::MaxChargeValue : maxChargeValue.toDouble();

	QString round = aSettings.get<QString>("<xmlattr>.round", "bank").toLower();
	if (round == "high")
	{
		commission.mRound = High;
	}
	else if (round == "low")
	{
		commission.mRound = Low;
	}
	else
	{
		commission.mRound = Bank;
	}

	commission.mBase = (aSettings.get<QString>("<xmlattr>.base", "amount_all").toLower() == "amount") ? Amount : AmountAll;

	return commission;
}

//----------------------------------------------------------------------------
Commission Commission::fromVariant(const QVariant & aCommissions)
{
	Commission commission;

	auto val = [&aCommissions](QString aName, QString aDefault) -> QVariant
	{
		QVariant v = aCommissions.toMap().value(aName);
		return v.isNull() ? aDefault : v;
	};

	commission.mType = val("type", QString()).toString().toLower() == "absolute" ? Absolute : Percent;
	commission.mValue = val("amount", QString()).toDouble();

	QString minChargeValue = val("min_charge", QString::number(CCommissions::MinChargeValue)).toString().trimmed();
	commission.mMinCharge = minChargeValue.isEmpty() ? CCommissions::MinChargeValue : minChargeValue.toDouble();

	QString maxChargeValue = val("max_charge", QString::number(CCommissions::MaxChargeValue)).toString().trimmed();
	commission.mMaxCharge = maxChargeValue.isEmpty() ? CCommissions::MaxChargeValue : maxChargeValue.toDouble();

	QString round = val("round", "bank").toString().toLower();
	if (round == "high")
	{
		commission.mRound = High;
	}
	else if (round == "low")
	{
		commission.mRound = Low;
	}
	else
	{
		commission.mRound = Bank;
	}

	commission.mBase = (val("base", "amount_all").toString().toLower() == "amount") ? Amount : AmountAll;

	return commission;
}

//----------------------------------------------------------------------------
CommissionList::CommissionList()
{
}

//----------------------------------------------------------------------------
TCommissions CommissionList::getCommissions() const
{
	return mCommissions;
}

//----------------------------------------------------------------------------
bool CommissionList::query(double aSum, Commission & aCommission) const
{
	bool response = true;

	foreach (const Commission & commission, mCommissions)
	{
		if (commission.contains(aSum))
		{
			if (response)
			{
				aCommission = commission;

				response = false;
			}
			else
			{
				if (commission > aCommission)
				{
					aCommission = commission;
				}
			}
		}
	}

	return !response;
}

//----------------------------------------------------------------------------
CommissionList & CommissionList::operator <<(const Commission & aCommission)
{
	mCommissions << aCommission;

	return *this;
}

//----------------------------------------------------------------------------
CommissionList CommissionList::fromSettings(const TPtree & aSettings) throw(std::runtime_error)
{
	CommissionList list;

	std::pair<TPtree::const_assoc_iterator, TPtree::const_assoc_iterator> searchBounds = aSettings.equal_range("amount");
	for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second; ++it)
	{
		Commission commission(Commission::fromSettings(it->second.get_child("commission")));

		commission.mAbove = it->second.get<double>("<xmlattr>.above", CCommissions::DefaultAboveValue);
		commission.mBelow = it->second.get<double>("<xmlattr>.below", CCommissions::DefaultBelowValue);

		list.mCommissions << commission;
	}

	searchBounds = aSettings.equal_range("commission");
	for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second; ++it)
	{
		list.mCommissions << Commission::fromSettings(it->second);
	}

	return list;
}

//----------------------------------------------------------------------------
CommissionList CommissionList::fromVariant(const QVariant & aCommissions)
{
	CommissionList list;
	QVariant amount = aCommissions.toMap().value("amount").toMap();	
	Commission commission(Commission::fromVariant(amount.toMap().value("commission")));

	commission.mAbove = amount.toMap().value("above", CCommissions::DefaultAboveValue).toDouble();
	commission.mBelow = amount.toMap().value("below", CCommissions::DefaultBelowValue).toDouble();
	
	list.mCommissions << commission;

	return list;
}

//----------------------------------------------------------------------------
CommissionByTimeList::CommissionByTimeList()
{
}

//----------------------------------------------------------------------------
TCommissions CommissionByTimeList::getCommissions() const
{
	QTime currentTime = QTime::currentTime();

	if ((currentTime >= mBegin) && (currentTime <= mEnd))
	{
		return mCommissions.getCommissions();
	}
	else
	{
		return TCommissions();
	}
}

//----------------------------------------------------------------------------
bool CommissionByTimeList::query(double aSum, SDK::PaymentProcessor::Commission & aCommission) const
{
	QTime time(QTime::currentTime());

	if ((mBegin <= time) && (time <= mEnd))
	{
		return mCommissions.query(aSum, aCommission);
	}

	return false;
}

//----------------------------------------------------------------------------
CommissionByTimeList & CommissionByTimeList::operator <<(const Commission & aCommission)
{
	mCommissions << aCommission;

	return *this;
}

//----------------------------------------------------------------------------
CommissionByTimeList CommissionByTimeList::fromSettings(const TPtree & aSettings)
{
	CommissionByTimeList list;

	list.mBegin = QTime::fromString(aSettings.get<QString>("<xmlattr>.begin"), "hh:mm:ss");
	list.mEnd = QTime::fromString(aSettings.get<QString>("<xmlattr>.end"), "hh:mm:ss");
	list.mCommissions = CommissionList::fromSettings(aSettings);

	return list;
}

//----------------------------------------------------------------------------
CommissionByTimeList CommissionByTimeList::fromVariant(const QVariant & aCommissions)
{
	Q_UNUSED(aCommissions);
	
	//TODO-TODO
	CommissionByTimeList list;
	return list;
}

//----------------------------------------------------------------------------
CommissionByDayList::CommissionByDayList()
{
}

//----------------------------------------------------------------------------
TCommissions CommissionByDayList::getCommissions() const
{
	TCommissions result;

	if (mDays.contains(static_cast<Commission::Day>(QDate::currentDate().dayOfWeek())))
	{
		foreach (const CommissionByTimeList & commissionByTime, mCommissionsByTime)
		{
			result = commissionByTime.getCommissions();

			if (!result.isEmpty())
			{
				break;
			}
		}

		if (result.isEmpty())
		{
			result = mCommissions.getCommissions();
		}
	}

	return result;
}

//----------------------------------------------------------------------------
bool CommissionByDayList::query(double aSum, SDK::PaymentProcessor::Commission & aCommission) const
{
	if (mDays.contains(static_cast<Commission::Day>(QDate::currentDate().dayOfWeek())))
	{
		foreach (const CommissionByTimeList & commissionByTime, mCommissionsByTime)
		{
			if (commissionByTime.query(aSum, aCommission))
			{
				return true;
			}
		}

		return mCommissions.query(aSum, aCommission);
	}

	return false;
}

//----------------------------------------------------------------------------
CommissionByDayList CommissionByDayList::fromSettings(const TPtree & aSettings)
{
	CommissionByDayList list;

	QStringList days = aSettings.get<QString>("<xmlattr>.id").split(QRegExp("\\s*,\\s*"));

	foreach (const QString & day, days)
	{
		int temp = day.toInt();

		if ((temp >= Commission::Mon) && (temp <= Commission::Sun))
		{
			list.mDays << static_cast<Commission::Day>(temp);
		}
	}

	std::pair<TPtree::const_assoc_iterator, TPtree::const_assoc_iterator> searchBounds = aSettings.equal_range("time");
	for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second; ++it)
	{
		list.mCommissionsByTime << CommissionByTimeList::fromSettings(it->second);
	}

	list.mCommissions = CommissionList::fromSettings(aSettings);

	return list;
}

//----------------------------------------------------------------------------
CommissionByDayList CommissionByDayList::fromVariant(const QVariant & aCommissions)
{
	CommissionByDayList list;

	QStringList days = aCommissions.toMap().value("day").toStringList();

	foreach(const QString & day, days)
	{
		int temp = day.toInt();

		if ((temp >= Commission::Mon) && (temp <= Commission::Sun))
		{
			list.mDays << static_cast<Commission::Day>(temp);
		}
	}

	list.mCommissions = CommissionList::fromVariant(aCommissions);

	return list;
}

//----------------------------------------------------------------------------
ProcessingCommission::ProcessingCommission()
	: mValue(0),
	  mMinValue(0),
	  mType(Real)
{
}

//----------------------------------------------------------------------------
ProcessingCommission ProcessingCommission::fromSettings(const TPtree & aSettings)
{
	ProcessingCommission result;

	auto settings = aSettings.get_child_optional("cyberaddcomission");
	
	if (settings.is_initialized())
	{
		result.mValue = settings->get<double>("<xmlattr>.percent");
		result.mMinValue = settings->get<double>("<xmlattr>.min_value");
		result.mType = static_cast<Type>(settings->get<int>("<xmlattr>.type"));
	}

	return result;
}

//----------------------------------------------------------------------------
ProcessingCommission ProcessingCommission::fromVariant(const QVariant & aCommissions)
{
	ProcessingCommission result;

	auto settings = aCommissions.toMap().value("cyberaddcomission");

	if (settings.isValid())
	{
		result.mValue = settings.toMap().value("percent").toDouble();
		result.mMinValue = settings.toMap().value("min_value").toDouble();
		result.mType = static_cast<Type>(settings.toMap().value("type").toInt());
	}

	return result;
}

//----------------------------------------------------------------------------
double ProcessingCommission::getValue(double aAmount, double aAmountAll)
{
	double result = 0.f;

	switch (mType)
	{
		case Real: result = aAmount * mValue / 100.0; break;
		case Diff: result = aAmountAll < aAmount ? 0.0 : (aAmountAll - aAmount) * mValue / 100.0; break;
		case Inverse: result = aAmount * mValue / (qFuzzyIsNull(100.0 - mValue) ? 1.0 : (100.0 - mValue)); break;
	}

	return qRound((result < mMinValue ? mMinValue : result) * 100.0) / 100.0;
}

//----------------------------------------------------------------------------
bool ProcessingCommission::isNull() const
{
	return qFuzzyIsNull(mValue) && qFuzzyIsNull(mMinValue);
}

//----------------------------------------------------------------------------
Commissions::Commissions()
	: mIsValid(false)
{
}

//----------------------------------------------------------------------------
bool Commissions::SComplexCommissions::sortByMinLimit(const Commission & aFirst, const Commission & aSecond)
{
	return aFirst.getMinLimit() < aSecond.getMinLimit();
}

//----------------------------------------------------------------------------
TCommissions Commissions::SComplexCommissions::getCommissions() const
{
	TCommissions result;

	foreach (const CommissionByDayList & commissionByDay, commissionsByDay)
	{
		result << commissionByDay.getCommissions();
	}

	if (result.isEmpty())
	{
		foreach (const CommissionByTimeList & commissionByTime, commissionsByTime)
		{
			result << commissionByTime.getCommissions();
		}

		if (result.isEmpty())
		{
			result = comissions.getCommissions();
		}
	}

	qSort(result.begin(), result.end(), &Commissions::SComplexCommissions::sortByMinLimit);

	return result;
}

//----------------------------------------------------------------------------
Commission Commissions::SComplexCommissions::query(double aSum) const
{
	Commission result;

	foreach (const CommissionByDayList & commissionByDay, commissionsByDay)
	{
		if (commissionByDay.query(aSum, result))
		{
			return result;
		}
	}

	foreach (const CommissionByTimeList & commissionByTime, commissionsByTime)
	{
		if (commissionByTime.query(aSum, result))
		{
			return result;
		}
	}

	comissions.query(aSum, result);

	return result;
}

//----------------------------------------------------------------------------
TCommissions Commissions::getCommissions(qint64 aProvider) const
{
	if (mProviderCommissions.contains(aProvider))
	{
		return mProviderCommissions[aProvider].getCommissions();
	}

	return mDefaultCommissions.getCommissions();
}

//----------------------------------------------------------------------------
Commission Commissions::getCommission(qint64 aProvider, double aSum) const
{
	if (mProviderCommissions.contains(aProvider))
	{
		return mProviderCommissions[aProvider].query(aSum);
	}

	return mDefaultCommissions.query(aSum);
}

//----------------------------------------------------------------------------
ProcessingCommission Commissions::getProcessingCommission(qint64 aProvider)
{
	return mProcessingCommissions.contains(aProvider) ? mProcessingCommissions[aProvider] : ProcessingCommission();
}

//----------------------------------------------------------------------------
int Commissions::getVAT(qint64 aProvider)
{
	if (mProviderCommissions.contains(aProvider))
	{
		return mProviderCommissions[aProvider].vat;
	}

	return 0;
}

//----------------------------------------------------------------------------
bool Commissions::isValid() const
{
	return mIsValid;
}

//----------------------------------------------------------------------------
bool Commissions::contains(qint64 aProvider, bool aCheckProcessing)
{
	return aCheckProcessing ? mProcessingCommissions.contains(aProvider) : mProviderCommissions.contains(aProvider);
}

//----------------------------------------------------------------------------
Commissions Commissions::fromSettings(const TPtree & aSettings)
{
	Commissions result;

	std::pair<TPtree::const_assoc_iterator, TPtree::const_assoc_iterator> searchBounds = aSettings.equal_range("operator");
	for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second; ++it)
	{
		try
		{
			result.mProviderCommissions.insert(it->second.get<qint64>("<xmlattr>.id"), result.loadCommissions(it->second));

			ProcessingCommission processingCommission = ProcessingCommission::fromSettings(it->second);
			if (!processingCommission.isNull())
			{
				result.mProcessingCommissions.insert(it->second.get<qint64>("<xmlattr>.id"), processingCommission);
			}
		}
		catch (std::runtime_error &)
		{
		}
	}

	result.mDefaultCommissions = result.loadCommissions(aSettings);
	result.mIsValid = true;

	return result;
}

//----------------------------------------------------------------------------
SDK::PaymentProcessor::Commissions Commissions::fromVariant(const QVariantList & aCommissions)
{
	Commissions result;

	foreach (QVariant com, aCommissions)
	{
		result.mProviderCommissions.insert(com.toMap()["provider"].toInt(), result.loadCommissions(com));
		
		ProcessingCommission processingCommission = ProcessingCommission::fromVariant(com);

		if (!processingCommission.isNull())
		{
			result.mProcessingCommissions.insert(com.toMap()["provider"].toInt(), processingCommission);
		}
	}

	result.mDefaultCommissions = Commissions::SComplexCommissions();
	result.mIsValid = true;

	return result;
}

//----------------------------------------------------------------------------
Commissions::SComplexCommissions Commissions::loadCommissions(const TPtree & aBranch)
{
	SComplexCommissions result;

	result.vat = aBranch.get<int>("vat", 0);

	std::pair<TPtree::const_assoc_iterator, TPtree::const_assoc_iterator> searchBounds = aBranch.equal_range("day");
	for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second; ++it)
	{
		result.commissionsByDay << CommissionByDayList::fromSettings(it->second);
	}

	searchBounds = aBranch.equal_range("time");
	for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second; ++it)
	{
		result.commissionsByTime << CommissionByTimeList::fromSettings(it->second);
	}

	result.comissions = CommissionList::fromSettings(aBranch);

	return result;
}

//----------------------------------------------------------------------------
Commissions::SComplexCommissions Commissions::loadCommissions(const QVariant & aCommissions)
{
	SComplexCommissions result;

	result.vat = aCommissions.toMap().value("vat", 0).toDouble();

	foreach (QVariant com, aCommissions.toMap().value("commissions").toList())
	{
		result.commissionsByDay << CommissionByDayList::fromVariant(com);
	}

	return result;
}

//----------------------------------------------------------------------------
void Commissions::appendFromSettings(const TPtree & aSettings)
{
	auto localComissions = fromSettings(aSettings);

	// Комиссии провайдера.
	{
		QMapIterator<qint64, SComplexCommissions> i(localComissions.mProviderCommissions);
		while (i.hasNext()) 
		{
			i.next();
		
			if (!this->mProviderCommissions.contains(i.key()))
			{
				this->mProviderCommissions.insert(i.key(), i.value());
			}
		}
	}

	// Комиссии процессинга.
	{
		QMapIterator<qint64, ProcessingCommission> i(localComissions.mProcessingCommissions);
		while (i.hasNext()) 
		{
			i.next();

			if (!this->mProcessingCommissions.contains(i.key()))
			{
				this->mProcessingCommissions.insert(i.key(), i.value());
			}
		}
	}
}

//----------------------------------------------------------------------------
void Commissions::clear()
{
	mIsValid = false;

	mProviderCommissions.clear();
	mProcessingCommissions.clear();
}

//----------------------------------------------------------------------------
}} // SDK::PaymentProcessor
