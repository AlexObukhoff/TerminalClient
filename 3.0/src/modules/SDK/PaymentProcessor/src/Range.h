/* @file Диапазон номерной ёмкости. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVector>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

//----------------------------------------------------------------------------
/// Диапазон номерной ёмкости.
struct SRange
{
	/// Границы диапазона.
	qint64 from;
	qint64 to;

	/// cid и id операторов.
	QVector<qint64> cids;
	QVector<qint64> ids;

	/// Оператор сравнения для сортировки списка из SRange.
	bool operator < (const SRange & aRange) const;
};

//---------------------------------------------------------------------------
/// Оператор сравнения для поиска диапазона в который входит aNumber c помощью qLowerBound().
bool operator < (const SRange & aRange, qint64 aNumber);

//---------------------------------------------------------------------------
/// Оператор сравнения для поиска диапазона в который входит aNumber с помощью qUpperBound().
bool operator < (qint64 aNumber, const SRange & aRange);

}} // SDK::PaymentProcessor

//---------------------------------------------------------------------------
