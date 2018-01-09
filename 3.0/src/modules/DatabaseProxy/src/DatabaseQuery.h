#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtSql/QtSql>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILog.h>

#include <DatabaseProxy/IDatabaseQuery.h>

//---------------------------------------------------------------------------
class IDatabaseQueryChecker;

//---------------------------------------------------------------------------
class DatabaseQuery : public IDatabaseQuery, public QSqlQuery
{
public:
	DatabaseQuery(QSqlDatabase db, IDatabaseQueryChecker * aQueryChecker);
	virtual ~DatabaseQuery();

	virtual bool prepare(const QString& aQuery);
	virtual void bindValue(const QString& aName, const QVariant& aValue);
	virtual void bindValue(int aPos, const QVariant& aValue);
	virtual bool exec();
	virtual void clear();

	virtual bool first();
	virtual bool next();
	virtual bool last();

	virtual bool isValid();

	virtual QVariant value(int i) const;

private:
	ILog * m_log;
	IDatabaseQueryChecker * mQueryChecker;
};

//---------------------------------------------------------------------------
