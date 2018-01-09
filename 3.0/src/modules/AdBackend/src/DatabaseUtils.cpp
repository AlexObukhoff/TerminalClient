/* @file Реализация базы данных. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QVariant>
#include <QtCore/QVariantList>
#include <QtCore/QResource>
#include <QtCore/QDir>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <Common/QtHeadersEnd.h>

// Проект
#include "DatabaseUtils.h"

//------------------------------------------------------------------------
inline void initMyResource() { Q_INIT_RESOURCE(BackendResources); }

//------------------------------------------------------------------------
namespace Ad
{

namespace CDatabaseUtils
{
	const QString Connection = "ad_statistics.connection";
	const QString Name = "ad_statistics.db";

	const QString ScriptPath = ":/database/empty_db.sql";
	const QString AddStatRecordQuery = "UPDATE ad_statistics SET quantity = quantity + 1 WHERE id = ? AND channel = ? AND `date` = CURRENT_DATE;";
	const QString SetStatRecordQuery = "UPDATE ad_statistics SET quantity = ? WHERE id = ? AND channel = ? AND `date` = CURRENT_DATE;";
	const QString CreateStatRecordQuery = "insert into ad_statistics(id, channel) VALUES(?, ?);";
}

//------------------------------------------------------------------------
DatabaseUtils::DatabaseUtils(const QString & aWorkingDirectory, ILog * aLog)
	: ILogable(aLog)
{
	initMyResource();

	mDatabase = QSqlDatabase::addDatabase("QSQLITE", CDatabaseUtils::Connection);
	mDatabase.setDatabaseName(aWorkingDirectory + QDir::separator() + CDatabaseUtils::Name);

	if (!mDatabase.open())
	{
		toLog(LogLevel::Error, QString("Failed to open database: %1").arg(mDatabase.lastError().text()));
	}
	else
	{
		QFile script(QResource(CDatabaseUtils::ScriptPath).absoluteFilePath());
		if (script.open(QIODevice::ReadOnly))
		{
			QList<QByteArray> lines = script.readAll().split('\n');
			QString cleanScript;

			foreach (QString line, lines)
			{
				line = line.simplified();
				if (!line.startsWith("--"))
				{
					cleanScript += line + " ";
				}
			}

			QStringList statements = cleanScript.split(";");
			foreach (const QString & line, statements)
			{
				QString statement = line.simplified();
				if (!statement.isEmpty())
				{
					QSqlQuery query = mDatabase.exec(statement);
					if (!query.isActive())
					{
						toLog(LogLevel::Error, 
							QString("Failed to create database: %1").arg(query.lastError().text()));
					}
				}
			}
		}
		else
		{
			toLog(LogLevel::Error, QString("Failed to open database sql script: %1").arg(script.errorString()));
		}
	}
}

//------------------------------------------------------------------------
DatabaseUtils::~DatabaseUtils()
{
}

//------------------------------------------------------------------------
bool DatabaseUtils::addStatisticRecord(qint64 aId, const QString & aChannel)
{
	bool result = false;

	if (!mDatabase.isOpen())
	{
		return false;
	}

	QSqlQuery query(mDatabase);
	query.prepare(CDatabaseUtils::AddStatRecordQuery);
	query.addBindValue(aId);
	query.addBindValue(aChannel);
	query.exec();

	result = query.isActive();

	if (!result)
	{
		toLog(LogLevel::Error, QString("Failed to execute query '%1': %2").arg(query.lastQuery()).arg(query.lastError().text()));
	}

	if (query.numRowsAffected() == 0)
	{
		QSqlQuery createQuery(mDatabase);
		createQuery.prepare(CDatabaseUtils::CreateStatRecordQuery);
		createQuery.addBindValue(aId);
		createQuery.addBindValue(aChannel);
		createQuery.exec();

		result = createQuery.isActive();

		if (!result)
		{
			toLog(LogLevel::Error, QString("Failed to execute query '%1': %2").arg(createQuery.lastQuery()).arg(createQuery.lastError().text()));
		}
	}

	return result;
}

//------------------------------------------------------------------------
bool DatabaseUtils::setStatisticRecord(qint64 aId, const QString & aChannel, int aValue)
{
	bool result = false;

	if (!mDatabase.isOpen())
	{
		return false;
	}

	QSqlQuery query(mDatabase);
	query.prepare(CDatabaseUtils::SetStatRecordQuery);
	query.addBindValue(aValue);
	query.addBindValue(aId);
	query.addBindValue(aChannel);
	query.exec();

	result = query.isActive();

	if (!result)
	{
		toLog(LogLevel::Error, QString("Failed to execute query '%1': %2").arg(query.lastQuery()).arg(query.lastError().text()));
	}

	if (query.numRowsAffected() == 0)
	{
		QSqlQuery createQuery(mDatabase);
		createQuery.prepare(CDatabaseUtils::CreateStatRecordQuery);
		createQuery.addBindValue(aId);
		createQuery.addBindValue(aChannel);
		createQuery.exec();

		result = createQuery.isActive();

		if (!result)
		{
			toLog(LogLevel::Error, QString("Failed to execute query '%1': %2").arg(createQuery.lastQuery()).arg(createQuery.lastError().text()));
		}
	}

	return result;
}
//------------------------------------------------------------------------
bool DatabaseUtils::getUnsentStatisticRecords(QList<SStatisticRecord> & aRecords, int aLimit)
{
	QSqlQuery query(mDatabase);

	if (!query.prepare("SELECT record_id, id, channel, date, quantity FROM ad_statistics WHERE quantity <> quantity_reported AND id > 0 LIMIT :limit"))
	{
		toLog(LogLevel::Error, QString("Cannot get unsent statistic records. Error: %1.")
			.arg(query.lastError().databaseText()));

		return false;
	}

	query.bindValue(":limit", aLimit);

	if (!query.exec())
	{
		toLog(LogLevel::Error, QString("Cannot get unsent statistic records. Error: %1.").arg(query.lastError().databaseText()));

		return false;
	}

	query.first();

	while (query.isValid())
	{
		aRecords <<
			SStatisticRecord(
				query.value(0).toLongLong(),
				query.value(1).toLongLong(),
				query.value(2).toString(),
				query.value(3).toDate(),
				query.value(4).toInt());

		query.next();
	}

	return true;
}

//------------------------------------------------------------------------
bool DatabaseUtils::deleteStatisticRecords(const QList<SStatisticRecord> & aRecords)
{
	QVariantList reportedList;
	QVariantList idList;

	foreach (SStatisticRecord record, aRecords)
	{
		reportedList << record.duration;
		idList << record.recordId;
	}

	QSqlQuery query(mDatabase);

	if (!query.prepare("UPDATE ad_statistics SET quantity_reported = ? WHERE record_id = ?"))
	{
		toLog(LogLevel::Error, QString("Cannot delete unsent statistic records. Error: %1.").arg(query.lastError().databaseText()));

		return false;
	}

	query.addBindValue(reportedList);
	query.addBindValue(idList);

	if (!query.execBatch())
	{
		toLog(LogLevel::Error, QString("Cannot delete unsent statistic records. Error: %1.").arg(query.lastError().databaseText()));

		return false;
	}

	return true;
}

//------------------------------------------------------------------------
} // namespace Ad
