/* @file Абстрактный запрос к серверу. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Project
#include "Request.h"

namespace SDK {
namespace PaymentProcessor {
namespace CyberPlat {

//---------------------------------------------------------------------------
Request::Request()
	: mIsOk(true),
	  mIsCriticalError(false)
{
}

//---------------------------------------------------------------------------
Request::~Request()
{
}

//---------------------------------------------------------------------------
bool Request::isOk() const
{
	return mIsOk;
}

//---------------------------------------------------------------------------
bool Request::isCriticalError() const
{
	return mIsCriticalError;
}

//---------------------------------------------------------------------------
void Request::addParameter(const QString & aName, const QVariant & aValue, const QVariant & aLogValue /*= QVariant()*/)
{
	mParameters[aName] = aValue;

	if (!aLogValue.isNull())
	{
		mLogParameters[aName] = aLogValue;
	}
}

//---------------------------------------------------------------------------
void Request::addRawParameter(const QString & aName, const QVariant & aValue, const QVariant & aLogValue /*= QVariant()*/)
{
	mRawParameters[aName] = aValue;

	if (!aLogValue.isNull())
	{
		mRawLogParameters[aName] = aLogValue;
	}
}

//---------------------------------------------------------------------------
void Request::removeParameter(const QString & aName, bool aRAWParameter /*= false*/)
{
	if (aRAWParameter)
	{
		mRawParameters.remove(aName);
	}
	else
	{
		mParameters.remove(aName);
	}
}

//---------------------------------------------------------------------------
QVariant Request::getParameter(const QString & aName, bool aRAWParameter /*= false*/) const
{
	return aRAWParameter ? mRawParameters.value(aName, QVariant()) : mParameters.value(aName, QVariant());
}

//---------------------------------------------------------------------------
const QVariantMap & Request::getParameters(bool aRAWParameter /*= false*/) const
{
	return aRAWParameter ? mRawParameters : mParameters;
}

//---------------------------------------------------------------------------
void Request::clear()
{
	mParameters.clear();
	mLogParameters.clear();
	mRawParameters.clear();
	mRawLogParameters.clear();
}

//---------------------------------------------------------------------------
QString Request::toString() const
{
	QString result;

	for (auto it = mParameters.begin(); it != mParameters.end(); ++it)
	{
		result += QString("%1=%2\r\n").arg(it.key()).arg(it.value().toString());
	}

	return result;
}

//---------------------------------------------------------------------------
QByteArray Request::toByteArray() const
{
	QByteArray result;

	for (auto it = mParameters.begin(); it != mParameters.end(); ++it)
	{
		result += it.key().toLatin1() + "=" + it.value().toByteArray()  + "\r\n";
	}

	return result;
}

//---------------------------------------------------------------------------
QString Request::toLogString() const
{
	QStringList result;

	for (auto it = mParameters.begin(); it != mParameters.end(); ++it)
	{
		result << QString("%1 = \"%2\"").arg(it.key()).arg(mLogParameters.contains(it.key()) ? mLogParameters.value(it.key()).toString() : it.value().toString());
	}

	for (auto it = mRawParameters.begin(); it != mRawParameters.end(); ++it)
	{
		result << QString("RAW(%1) = \"%2\"").arg(it.key()).arg(mRawLogParameters.contains(it.key()) ? mRawLogParameters.value(it.key()).toString() : it.value().toString());
	}

	return result.join(", ");
}

//------------------------------------------------------------------------------
}}} // SDK::PaymentProcessor::CyberPlat
