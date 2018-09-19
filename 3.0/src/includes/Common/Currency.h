/* @file Класс описывающий номинал валюты. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QHash>
#include <QtCore/qmath.h>
#include <Common/QtHeadersEnd.h>


namespace Currency
{
	
//--------------------------------------------------------------------------------
/// Класс для описания номинала купюры.
class Nominal
{
public:
	typedef int RawType;

	explicit Nominal(int aValue) : mNominal(aValue * 100) { }
	explicit Nominal(double aValue) : mNominal(qFloor((aValue * 1000 + 0.001) / 10.0)) { }

	static Nominal fromRawValue(RawType aRawValue) { Nominal n(0); n.mNominal = aRawValue; return n; }

	operator int() const { return mNominal / 100; }
	operator double() const { return toDouble(); }

	RawType rawValue() const { return mNominal; }
	double toDouble() const { return mNominal / 100.; }

	bool operator ==(const Nominal & aNominal) const { return this->mNominal == aNominal.mNominal; }
	bool operator <(const Nominal & aNominal) const { return this->mNominal < aNominal.mNominal; }
	bool operator >=(const Nominal & aNominal) const { return this->mNominal >= aNominal.mNominal; }

	const Nominal & operator =(int aNominal) { mNominal = aNominal * 100; return *this; }
	const Nominal & operator =(double aNominal) { mNominal =  qFloor(aNominal * 100); return *this; }

	QString toString(bool aTrimZeroFraction = true) const
	{
		return (mNominal % 100) == 0 && aTrimZeroFraction ? QString::number(mNominal / 100) : QString::number(mNominal / 100., 'f', 2);
	}

private:
	RawType mNominal;
};

} // end namespace Currency

//--------------------------------------------------------------------------------
inline uint qHash(const Currency::Nominal & aValue) { return qHash(aValue.rawValue()); }

//--------------------------------------------------------------------------------
