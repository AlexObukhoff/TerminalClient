// Qt
#include "Common/QtHeadersBegin.h"
#include <QtCore/QVariant>
#include <QtCore/QDateTime>
#include <QtCore/QMutexLocker>
#include "Common/QtHeadersEnd.h"

// Modules
#include <Common/ExceptionFilter.h>
#include <Common/ILog.h>

// Project
#include "DatabaseQuery.h"
#include "MySqlDatabaseProxy.h"

//---------------------------------------------------------------------------
MySqlDatabaseProxy::MySqlDatabaseProxy()
: mMutex(QMutex::Recursive), mDb(nullptr), mQueryChecker(nullptr)
{
	mLog = ILog::getInstance(CMySqlDatabaseProxy::DefaultLog);
}

//---------------------------------------------------------------------------
MySqlDatabaseProxy::~MySqlDatabaseProxy()
{
	delete mDb;
	mDb = NULL;
}

//---------------------------------------------------------------------------
void MySqlDatabaseProxy::setQueryChecker(IDatabaseQueryChecker * aQueryChecker)
{
	mQueryChecker = aQueryChecker;
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::open(const QString& dbName,
                              const QString& user,
                              const QString& password,
                              const QString& host,
                              const int aPort)
{
	try
	{
		mDb = new QSqlDatabase(QSqlDatabase::addDatabase(CMySqlDatabaseProxy::DriverName, CMySqlDatabaseProxy::ConnactionName));

		mCurrentBase = dbName;

		mDb->setDatabaseName(dbName);
		mDb->setUserName(user);
		mDb->setPassword(password);
		mDb->setHostName(host);
		mDb->setPort(aPort);

		bool openResult = mDb->open();

		if (openResult)
		{
			LOG(mLog, LogLevel::Normal, QString("Database has been opened: %1.").arg(dbName));
			return true;
		}
		else
		{
			LOG(mLog, LogLevel::Error, QString("Folowing error occured: %1.").arg(mDb->lastError().driverText()));
		}
	}
	catch (...)
	{
		EXCEPTION_FILTER(mLog);
	}
	
	LOG(mLog, LogLevel::Fatal, QString("Can't open database: %1.").arg(dbName));
	
	return false;
}

//---------------------------------------------------------------------------
void MySqlDatabaseProxy::close()
{
	if (!mDb)
	{
		return;
	}

	if (mDb->isOpen())
	{
		mDb->close();
	}

	delete mDb;
	mDb = NULL;

	QSqlDatabase::removeDatabase(CMySqlDatabaseProxy::ConnactionName);

	LOG(mLog, LogLevel::Normal, QString("Database has been closed: %1.").arg(mCurrentBase));
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::isConnected() const
{
	return mDb->isOpen();
}

//---------------------------------------------------------------------------
const QString& MySqlDatabaseProxy::getCurrentBaseName() const
{
	return mCurrentBase;
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::safeExec(QSqlQuery* query, const QString& queryMessage)
{
	QMutexLocker locker(&mMutex);

	try
	{
		if (!mQueryChecker->isGood(query->exec(queryMessage)))
		{
			LOG(mLog, LogLevel::Error, QString("Can't execute query: %1. Error: %2.")
				.arg(queryMessage)
				.arg(query->lastError().text()));

			return false;
		}
	}
	catch(...)
	{
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::execDML(const QString& strQuery, long& rowsAffected)
{
	if (!mDb)
	{
		return false;
	}

	QSqlQuery dbQuery(*mDb);

	bool execResult = safeExec(&dbQuery, strQuery);

	rowsAffected = dbQuery.numRowsAffected();

	return execResult;
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::execScalar(const QString& strQuery, long& result)
{
	if (!mDb)
	{
		return false;
	}

	QSqlQuery dbQuery(*mDb);

	result = 0;

	bool execResult = safeExec(&dbQuery, strQuery);

	if (dbQuery.first())
		result = static_cast<long>(dbQuery.value(0).toLongLong());

	return execResult;
}

//---------------------------------------------------------------------------
IDatabaseQuery* MySqlDatabaseProxy::execQuery(const QString& strQuery)
{
	if (!mDb)
	{
		return false;
	}

	QScopedPointer<IDatabaseQuery> dbQuery(new DatabaseQuery(*mDb, mQueryChecker));

	QSqlQuery* dbQtQuery = dynamic_cast<QSqlQuery*>(dbQuery.data());

	return safeExec(dbQtQuery, strQuery) ? dbQuery.take() : nullptr;
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::transaction()
{
	if (!mDb)
	{
		return false;
	}

	QMutexLocker locker(&mMutex);

	return mDb->transaction();
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::commit()
{
	if (!mDb)
	{
		return false;
	}

	QMutexLocker locker(&mMutex);

	return mDb->commit();
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::rollback()
{
	if (!mDb)
	{
		return false;
	}

	QMutexLocker locker(&mMutex);

	return mDb->rollback();
}

//---------------------------------------------------------------------------
IDatabaseQuery* MySqlDatabaseProxy::createQuery()
{
	if (!mDb)
	{
		throw std::runtime_error("cannot create query without valid database");
	}

	return new DatabaseQuery(*mDb, mQueryChecker);
}

//---------------------------------------------------------------------------
IDatabaseQuery * MySqlDatabaseProxy::createQuery(const QString & aQueryString)
{
	IDatabaseQuery * query = createQuery();

	if (!mQueryChecker->isGood(query->prepare(aQueryString)))
	{
		delete query;
		query = nullptr;
	}

	return query;
}

//---------------------------------------------------------------------------
bool MySqlDatabaseProxy::checkIntegrity(QStringList & aListErrors)
{
	Q_UNUSED(aListErrors)
	
	return true;
}

//---------------------------------------------------------------------------
