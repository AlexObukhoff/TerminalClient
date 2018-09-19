/* @brief Абстрактный запрос к СУБД. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
namespace CIDatabaseQuery
{
	const QString DefaultLog = "DatabaseProxy";
}

//---------------------------------------------------------------------------
class IDatabaseQuery
{
public:
	virtual ~IDatabaseQuery() {};

	virtual bool prepare(const QString& aQuery) = 0;
	virtual void bindValue(const QString& aName, const QVariant& aValue) = 0;
	virtual void bindValue(int aPos, const QVariant& aValue) = 0;
	virtual bool exec() = 0;
	virtual void clear() = 0;

	virtual bool first() = 0;
	virtual bool next() = 0;
	virtual bool last() = 0;

	virtual bool isValid() = 0;
	virtual int  numRowsAffected() const = 0;

	virtual QVariant value(int i) const = 0;
};
//---------------------------------------------------------------------------
