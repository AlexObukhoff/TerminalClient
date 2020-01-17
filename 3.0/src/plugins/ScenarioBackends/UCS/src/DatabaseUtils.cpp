/* @file Реализация утилиты доступа к БД для процессинга UCS. */

// Modules
#include <DatabaseProxy/IDatabaseProxy.h>
#include <DatabaseProxy/IDatabaseQuery.h>


// Project
#include "DatabaseUtils.h"

namespace UcsDB
{

const char * DateFormat = "yyyy-MM-dd hh:mm:ss";

//------------------------------------------------------------------------------
DatabaseUtils::DatabaseUtils(SDK::PaymentProcessor::IDatabaseService * aDatabaseService, ILog * aLog) :
	ILogable(aLog),
	mDatabase(aDatabaseService),
	mReadOnly(true)
{
	initTables();
}

//------------------------------------------------------------------------------
bool DatabaseUtils::isReadOnly() const
{
	return mReadOnly;
}

//------------------------------------------------------------------------------
bool DatabaseUtils::save(const Encashment & aEncashment)
{
	auto query = mDatabase->prepareQuery(
		"INSERT INTO [ucs_encashments] (`create_date`, `receipt`, `printed`) "
		"VALUES (:create_date, :receipt, :printed);");

	if (!query)
	{
		toLog(LogLevel::Error, QString("Failed to save aEncashment %1.").arg(aEncashment.date.toString()));

		return false;
	}

	query->bindValue(":create_date", aEncashment.date.toString(DateFormat));
	query->bindValue(":receipt", aEncashment.receipt.join("\n"));
	query->bindValue(":printed", aEncashment.printed);

	return mDatabase->execQuery(query);
}

//------------------------------------------------------------------------------
QList<UcsDB::Encashment> DatabaseUtils::getAllNotPrinted() const
{
	QList<UcsDB::Encashment> result;

	auto query = mDatabase->createAndExecQuery(
		"SELECT `create_date`, `receipt`, `printed` FROM [ucs_encashments] WHERE printed = 0;");

	if (query->first())
	{
		do
		{
			UcsDB::Encashment encashment;

			encashment.date = QDateTime::fromString(query->value(0).toString(), DateFormat);
			encashment.receipt = query->value(1).toString().split("\n");
			encashment.printed = query->value(2).toInt();

			result.push_back(encashment);
		} while (query->next());
	}

	return result;
}

//------------------------------------------------------------------------------
bool DatabaseUtils::markAsPrinted(const Encashment & aEncashment)
{
	auto query = mDatabase->prepareQuery(
		"UPDATE [ucs_encashments] SET `printed` = 1 WHERE `create_date` = :create_date;");

	if (!query)
	{
		toLog(LogLevel::Error, QString("Failed to update aEncashment %1.").arg(aEncashment.date.toString()));

		return false;
	}

	query->bindValue(":create_date", aEncashment.date.toString(DateFormat));

	return mDatabase->execQuery(query);
}

//------------------------------------------------------------------------------
bool DatabaseUtils::initTables()
{
	QStringList sqls;

	sqls
		<< "CREATE TABLE IF NOT EXISTS [ucs_encashments] ([create_date] DATETIME PRIMARY KEY, [receipt] TEXT NOT NULL, [printed] INT DEFAULT (0));"
		<< "CREATE INDEX IF NOT EXISTS [ucs_encashments_printed] ON [ucs_encashments] ([printed]);";

	// Создаём таблицы
	foreach(auto sql, sqls)
	{
		if (!mDatabase->execQuery(sql))
		{
			toLog(LogLevel::Error, QString("Failed to execute SQL: '%1'.").arg(sql));

			return false;
		}
	}

	mReadOnly = false;

	return true;
}

//------------------------------------------------------------------------------
Encashment::Encashment() :
	date(QDateTime::currentDateTime()),
	printed(0)
{
}

}

//------------------------------------------------------------------------------

