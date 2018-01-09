/* @file Диапазон номерной ёмкости. */

#include "Range.h"

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
bool SRange::operator < (const SRange & aRange) const
{
	return from < aRange.from;
}

//---------------------------------------------------------------------------
bool operator < (const SRange & aRange, qint64 aNumber)
{
	return aRange.to < aNumber;
}

//---------------------------------------------------------------------------
bool operator < (qint64 aNumber, const SRange & aRange)
{
	return aNumber < aRange.from;
}

}} // SDK::PaymentProcessor

//---------------------------------------------------------------------------
