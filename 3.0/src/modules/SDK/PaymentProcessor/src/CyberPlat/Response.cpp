/* @file Базовый ответ сервера. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Project
#include "Response.h"

namespace SDK {
namespace PaymentProcessor {
namespace CyberPlat {

//---------------------------------------------------------------------------
Response::Response(const Request & aRequest, const QString & aResponseString)
	: mResponseString(aResponseString),
	  mError(ELocalError::NetworkError),
	  mResult(EServerResult::Empty),
	  mRequest(aRequest)
{
	QRegExp regexp("^(\\w+)=(.*)$");
	regexp.setMinimal(true);

	foreach(auto line, mResponseString.split("\r\n"))
	{
		if(regexp.indexIn(line) != -1)
		{
			if (regexp.captureCount() > 1)
			{
				addParameter(regexp.cap(1), regexp.cap(2));
			}
		}
	}
}

//---------------------------------------------------------------------------
Response::~Response()
{
}

//---------------------------------------------------------------------------
bool Response::isOk()
{
	return ((mError == EServerError::Ok) && (mResult == EServerResult::Ok));
}

//---------------------------------------------------------------------------
int Response::getError() const
{
	return mError;
}

//---------------------------------------------------------------------------
int Response::getResult() const
{
	return mResult;
}

//---------------------------------------------------------------------------
const QString & Response::getErrorMessage() const
{
	return mErrorMessage;
}

//---------------------------------------------------------------------------
QVariant Response::getParameter(const QString & aName) const
{
	return mParameters.contains(aName) ? mParameters.value(aName) : QVariant();
}

//---------------------------------------------------------------------------
const QVariantMap & Response::getParameters() const
{
	return mParameters;
}

//---------------------------------------------------------------------------
const QString & Response::toString() const
{
	return mResponseString;
}

//---------------------------------------------------------------------------
QString Response::toLogString() const
{
	QStringList result;

	for (auto it = getParameters().begin(); it != getParameters().end(); ++it)
	{
		result << QString("%1 = \"%2\"").arg(it.key()).arg(it.value().toString());
	}

	return result.join(", ");
}

//---------------------------------------------------------------------------
void Response::addParameter(const QString & aName, const QString & aValue)
{
	mParameters[aName] = aValue;
	
	if (aName == CResponse::Parameters::Error)
	{
		mError = static_cast<EServerError::Enum>(aValue.toInt());
	}
	else if (aName == CResponse::Parameters::ErrorCode)
	{
		mError = static_cast<EServerError::Enum>(aValue.toInt());
	}

	if (aName == CResponse::Parameters::Result)
	{
		mResult = static_cast<EServerResult::Enum>(aValue.toInt());
	}

	if (aName == CResponse::Parameters::ErrorMessage)
	{
		mErrorMessage = aValue;
	}
}

//---------------------------------------------------------------------------
const Request & Response::getRequest() const
{
	return mRequest;
}

//------------------------------------------------------------------------------
}}} // SDK::PaymentProcessor::CyberPlat
