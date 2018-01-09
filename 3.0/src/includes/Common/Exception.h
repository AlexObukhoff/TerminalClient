/* @file Базовый класс исключений. */

#pragma once

// stl
#include <stdexcept>

// Qt
#include "Common/QtHeadersBegin.h"
#include <QtCore/QString>
#include "Common/QtHeadersEnd.h"

//--------------------------------------------------------------------------------
namespace ECategory
{
	enum Enum
	{
		System,
		Network
	};
}

//--------------------------------------------------------------------------------
namespace ESeverity
{
	enum Enum
	{
		Minor,
		Major,
		Critical
	};
}

//--------------------------------------------------------------------------------
/// Класс для представления исключений с поддержкой QString.
class Exception : public std::exception
{
public:
	Exception(ECategory::Enum aCategory, ESeverity::Enum aSeverity, int aCode, const QString & aMessage = QString())
		: mCategory(aCategory), mSeverity(aSeverity), mCode(aCode), mMessage(aMessage) {}

	Exception(const QString & aMessage)
		: mCategory(ECategory::System), mSeverity(ESeverity::Minor), mCode(0), mMessage(aMessage) {}

	virtual ~Exception() throw() {}
	
	ECategory::Enum getCategory() const { return mCategory; }
	ESeverity::Enum getSeverity() const { return mSeverity; }
	int getCode() const { return mCode; }
	QString getMessage() const { return mMessage; }

	virtual const char * what() const
	{
		return mMessage.toLocal8Bit();
	}

protected:
	ECategory::Enum mCategory;
	ESeverity::Enum mSeverity;
	int mCode;
	QString mMessage;
};

//--------------------------------------------------------------------------------
