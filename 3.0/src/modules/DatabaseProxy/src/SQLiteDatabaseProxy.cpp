
// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QMutexLocker>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/Application.h>
#include <Common/SleepHelper.h>

// Project
#include "DatabaseQuery.h"
#include "SQLiteDatabaseProxy.h"

//---------------------------------------------------------------------------
namespace CSQLiteDatabaseProxy
{
	const int MaxTryCount = 3;
}

//---------------------------------------------------------------------------
SQLiteDatabaseProxy::SQLiteDatabaseProxy()
	: ILogable(CIDatabaseProxy::LogName),
	  mMutex(QMutex::Recursive),
	  mQueryChecker(nullptr)
{
}

//---------------------------------------------------------------------------
SQLiteDatabaseProxy::~SQLiteDatabaseProxy()
{
}

//---------------------------------------------------------------------------
void SQLiteDatabaseProxy::setQueryChecker(IDatabaseQueryChecker * aQueryChecker)
{
	mQueryChecker = aQueryChecker;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::open(const QString& aDbName,
							   const QString& aUser,
							   const QString& aPassword,
							   const QString& aHost,
							   const int /*aPort*/)
{
	mCurrentBase = aDbName;

	if (QDir::isRelativePath(mCurrentBase))
	{
		mCurrentBase = QDir::cleanPath(BasicApplication::getInstance()->getWorkingDirectory() +
			QDir::separator() + mCurrentBase);
	}

	if (mDb && mDb->isOpen())
	{
		toLog(LogLevel::Normal, "Before open new database, current database must be closed.");

		close();
	}

	mDb = QSharedPointer<QSqlDatabase>(new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")));

	mDb->setDatabaseName(mCurrentBase);
	mDb->setUserName(aUser);
	mDb->setPassword(aPassword);
	mDb->setHostName(aHost);
	mDb->setConnectOptions(QString("QSQLITE_BUSY_TIMEOUT=%1").arg(CSQLiteDatabaseProxy::BusyTimeout));

	if (mDb->open())
	{
		toLog(LogLevel::Normal, QString("Database opened: %1.").arg(mCurrentBase));

		return true;
	}
	else
	{
		toLog(LogLevel::Error, QString("Cannot open database %1. Folowing error occured: %2.")
			.arg(mCurrentBase)
			.arg(mDb->lastError().driverText()));

		return false;
	}
}

//---------------------------------------------------------------------------
void SQLiteDatabaseProxy::close()
{
	if (isConnected())
	{
		mDb->close();
	}

	mDb.clear();
	mCurrentBase.clear();

	toLog(LogLevel::Normal, QString("Database has been closed: %1.").arg(mCurrentBase));
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::isConnected() const
{
	return (mDb && mDb->isOpen());
}

//---------------------------------------------------------------------------
const QString & SQLiteDatabaseProxy::getCurrentBaseName() const
{
	return mCurrentBase;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::safeExec(QSqlQuery * aQuery, const QString & aQueryMessage)
{
	QMutexLocker locker(&mMutex);

	int tryCount = 0;

	while (tryCount < CSQLiteDatabaseProxy::MaxTryCount)
	{
		if (!aQuery->exec(aQueryMessage))
		{
			mQueryChecker->isGood(!aQuery->lastError().isValid() || aQuery->lastError().type() == QSqlError::NoError);

			toLog(LogLevel::Error, QString("Can't execute query: %1. Error: %2.")
				.arg(aQueryMessage)
				.arg(aQuery->lastError().text()));

			if (aQuery->lastError().text().contains("disk I/O error"))
			{
				toLog(LogLevel::Normal, "Trying to recover after database disk I/O error.");

				SleepHelper::msleep(100);
			}
			else
			{
				return false;
			}
		}
		else
		{
			break;
		}

		++tryCount;
	}

	return true;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::execDML(const QString & aQuery, long & aRowsAffected)
{
	QMutexLocker locker(&mMutex);

	if (!isConnected())
	{
		toLog(LogLevel::Error, QString("Cannot execute sql query in unconnected state. Query: %1.")
			.arg(aQuery));

		return false;
	}

	QSqlQuery dbQuery(*mDb);

	if (safeExec(&dbQuery, aQuery))
	{
		aRowsAffected = dbQuery.numRowsAffected();

		return true;
	}
	
	return false;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::execScalar(const QString & aQuery, long & aResult)
{
	QMutexLocker locker(&mMutex);

	if (!isConnected())
	{
		toLog(LogLevel::Error, QString("Cannot execute sql query in unconnected state. Query: %1.").arg(aQuery));

		return false;
	}

	QSqlQuery dbQuery(*mDb);

	if (safeExec(&dbQuery, aQuery) && dbQuery.first() && dbQuery.record().count())
	{
		aResult = static_cast<long>(dbQuery.value(0).toLongLong());

		return true;
	}

	return false;
}

//---------------------------------------------------------------------------
IDatabaseQuery * SQLiteDatabaseProxy::execQuery(const QString & aQuery)
{
	QMutexLocker locker(&mMutex);

	if (!isConnected())
	{
		toLog(LogLevel::Error, QString("Cannot execute sql query in unconnected state. Query: %1.").arg(aQuery));

		return false;
	}

	QScopedPointer<IDatabaseQuery> dbQuery(createQuery());

	QSqlQuery * dbQtQuery = dynamic_cast<QSqlQuery*>(dbQuery.data());

	if (safeExec(dbQtQuery, aQuery))
	{
		return dbQuery.take();
	}

	return nullptr;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::transaction()
{
	if (!isConnected())
	{
		toLog(LogLevel::Error, "Cannot start transaction in unconnected state.");

		return false;
	}

	if (!mQueryChecker->isGood(mDb->transaction()))
	{
		toLog(LogLevel::Error, QString("Cannot start transaction. Error: %1.").arg(mDb->lastError().text()));

		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::commit()
{
	if (!isConnected())
	{
		toLog(LogLevel::Error, "Cannot commit transaction in unconnected state.");

		return false;
	}

	if (!mQueryChecker->isGood(mDb->commit()))
	{
		toLog(LogLevel::Error, QString("Cannot commit transaction. Error: %1.").arg(mDb->lastError().text()));

		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
bool SQLiteDatabaseProxy::rollback()
{
	if (!isConnected())
	{
		toLog(LogLevel::Error, "Cannot rollback transaction in unconnected state.");

		return false;
	}

	if (!mQueryChecker->isGood(mDb->rollback()))
	{
		toLog(LogLevel::Error, QString("Cannot rollback transaction. Error: %1.").arg(mDb->lastError().text()));

		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
IDatabaseQuery * SQLiteDatabaseProxy::createQuery()
{
	if (!isConnected())
	{
		toLog(LogLevel::Error, "Cannot create sql query in unconnected state.");

		return nullptr;
	}

	return new DatabaseQuery(*mDb, mQueryChecker);
}

//---------------------------------------------------------------------------
IDatabaseQuery * SQLiteDatabaseProxy::createQuery(const QString & aQueryString)
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
bool SQLiteDatabaseProxy::checkIntegrity(QStringList & aListErrors)
{
	QScopedPointer<IDatabaseQuery> query(createQuery());

	if (!query->prepare("PRAGMA integrity_check"))
	{
		aListErrors << dynamic_cast<DatabaseQuery *>(query.data())->lastError().databaseText();

		toLog(LogLevel::Error, "Failed to create query for integrity check.");

		return false;
	}

	if (!mQueryChecker->isGood(query->exec()))
	{
		aListErrors << dynamic_cast<DatabaseQuery *>(query.data())->lastError().databaseText();

		toLog(LogLevel::Error, "Failed exec integrity check.");

		return false;
	}

	bool result = true;

	for (query->first(); query->isValid(); query->next())
	{
		QString message = query->value(0).toString();
		if (message.compare("ok", Qt::CaseInsensitive) != 0)
		{
			aListErrors << message;

			toLog(LogLevel::Error, QString("DATABASE INTEGRITY FAILED: %1.").arg(message));

			result = false;
		}
	}

	return result;
}

//---------------------------------------------------------------------------
