/* @file Классы для хранения [описателей] данных. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
// базовый класс для хранения данных в виде пар ключ-значение
template <class T1, class T2>
class CSpecification
{
public:
	CSpecification(const T2 & aDefault = T2()) : mDefaultValue(aDefault) {}
	const T2 operator[](const T1 & aKey) const {return value(aKey);}
	T1 key(const T2 & aValue) {return mBuffer.key(aValue);}
	virtual T2 value(const T1 & aKey) const {return mBuffer.value(aKey, mDefaultValue);}
	void append(const T1 & aKey, const T2 & aParameter) {mBuffer.insert(aKey, aParameter);}
	QMap<T1, T2> & data() {return mBuffer;}
	void setDefault(const T2 & aDefaultValue) {mDefaultValue = aDefaultValue;}
	T2 getDefault() {return mDefaultValue;}

protected:
	QMap<T1, T2> mBuffer;
	T2 mDefaultValue;
};

// класс для хранения произвольных описателей данных
template <class T>
class CDescription : public CSpecification<T, QString>
{
public:
	void append(const T & aKey, const char * aParameter) {mBuffer.insert(aKey, QString::fromUtf8(aParameter));}
	void append(const T & aKey, const QString & aParameter) {mBuffer.insert(aKey, aParameter);}
};

// класс для хранения произвольных описателей данных в виде битовой маски
template <class T>
class CBitmapDescription : public CDescription<T>
{
public:
	virtual QString getValues(T aValue)
	{
		QStringList result;

		for (auto it = data().begin(); it != data().end(); ++it)
		{
			if (it.key() & aValue)
			{
				result << it.value();
			}
		}

		return result.join(", ");
	}

protected:
	void addBit(int aBit, const char * aParameter)
	{
		append(T(1) << aBit, QString::fromUtf8(aParameter));
	}
};

#define APPEND(aKey) append(aKey, #aKey)
#define ADD(aKey) add(aKey, #aKey)

// базовый класс для хранения данных в виде пар ключ-значение со статическим заполнением данных
template <class T1, class T2>
class CSSpecification: public CSpecification<T1, T2>
{
public:
	CSSpecification()
	{
		mBuffer = addData(T1(), T2(), true);
	}

	static QMap<T1, T2> addData(const T1 & aKey, const T2 & aValue, bool aMode = false)
	{
		static QMap<T1, T2> data;

		if (aMode)
		{
			return data;
		}

		data.insert(aKey, aValue);

		return QMap<T1, T2>();
	}
};

#define ADD_STATIC_DATA(aClass, T1, aName, aKey, aValue) const T1 aName = [] () -> T1 { aClass::addData(aKey, aValue); return aKey; } ();

//---------------------------------------------------------------------------
