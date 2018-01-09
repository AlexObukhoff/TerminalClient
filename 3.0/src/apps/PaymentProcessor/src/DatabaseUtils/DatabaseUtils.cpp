/* @file Реализация интерфейсов для работы с БД. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QScopedPointer>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtCore/QMutexLocker>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ExceptionFilter.h>

#include <DatabaseProxy/IDatabaseQuery.h>
#include <DatabaseProxy/IDatabaseProxy.h>

// Project
#include "System/IApplication.h"

#include "DatabaseUtils.h"

//---------------------------------------------------------------------------
namespace CDatabaseUtils
{
	const QString EmptyDatabaseScript = ":/scripts/empty_db.sql";
	const QString DatabasePatchParam  = "db_patch";
	const QString DatabaseUpdateParam = "db_update";

	// Список патчей базы
	struct
	{
		int version;
		QString Script;
	} Patches[] = {
		{ 6, ":/scripts/db_patch_6.sql" },
		{ 7, ":/scripts/db_patch_7.sql" },
		{ 8, ":/scripts/db_patch_8.sql" },
		{ 9, ":/scripts/db_patch_9.sql" },
		{ 10, ":/scripts/db_patch_10.sql" },
		{ 11, ":/scripts/db_patch_11.sql" },
		{ 12, ":/scripts/db_patch_12.sql" },
	};

}

//---------------------------------------------------------------------------
DatabaseUtils::DatabaseUtils(IDatabaseProxy & aProxy, IApplication * aApplication)
	: mDatabase(aProxy),
	  mApplication(aApplication),
	  mLog(aApplication->getLog()),
	  mPaymentLog(ILog::getInstance("Payments")),
	  mAccessMutex(QMutex::Recursive)
{
}

//---------------------------------------------------------------------------
DatabaseUtils::~DatabaseUtils()
{
}

//---------------------------------------------------------------------------
bool DatabaseUtils::initialize()
{
	try
	{
		if (!mDatabase.isConnected())
		{
			throw std::runtime_error("not connected.");
		}

		if (!mDatabase.transaction())
		{
			throw std::runtime_error("Cannot start database transaction.");
		}

		// Проверяем, созданы ли таблицы.
		if (!databaseTableCount())
		{
			updateDatabase(CDatabaseUtils::EmptyDatabaseScript);
			// после этого скрипта databasePatch() = 5
		}

		for (int i = 0; i < sizeof(CDatabaseUtils::Patches) / sizeof(CDatabaseUtils::Patches[0]); i++)
		{
			if (databasePatch() < CDatabaseUtils::Patches[i].version)
			{
				LOG(mLog, LogLevel::Normal, QString("Patch database to version %1.").arg(CDatabaseUtils::Patches[i].version));

				updateDatabase(CDatabaseUtils::Patches[i].Script);
			}
		}
	}
	catch (...)
	{
		EXCEPTION_FILTER(mLog);

		mDatabase.rollback();

		return false;
	}

	if (!mDatabase.commit())
	{
		LOG(mLog, LogLevel::Error, "Failed to commit changes to the terminal database.");
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
bool DatabaseUtils::updateDatabase(const QString & aSqlScriptName)
{
	QFile ftemp(aSqlScriptName);
	if (!ftemp.open(QIODevice::ReadOnly))
	{
		throw std::runtime_error("Cannot open database script from resources.");
	}

	QString resSQL = ftemp.readAll();

	// Удалим коментарии ("-- some comment") и ("/* many \n comment */").
	QRegExp rx("(\\/\\*.*\\*\\/|\\-\\-.*\\n)");
	rx.setMinimal(true);
	resSQL.replace(rx, "");

	// Удалим перевод строк с сохранением возможности писать ("CREATE\nTABLE")
	resSQL.replace("\r", "");
	resSQL.replace("\n", " ");

	QStringList creatingSteps = resSQL.split(";");

	long rowsAffected(0);

	foreach (const QString & step, creatingSteps)
	{
		if (step.trimmed().isEmpty())
		{
			continue;
		}

		if (!mDatabase.execDML(step, rowsAffected))
		{
			throw std::runtime_error("Cannot execute regular sql query.");
		}
	}

	return true;
}

//---------------------------------------------------------------------------
int DatabaseUtils::databaseTableCount() const
{
	QString queryMessage = "SELECT count(*) FROM sqlite_master WHERE type = 'table'";
	QScopedPointer<IDatabaseQuery> dbQuery(mDatabase.execQuery(queryMessage));
	if (!dbQuery)
	{
		LOG(mLog, LogLevel::Error, "Failed to check database tables presence.");
		return 0;
	}

	if (dbQuery->first())
	{
		return dbQuery->value(0).toInt();
	}

	return 0;
}

//---------------------------------------------------------------------------
int DatabaseUtils::databasePatch() const
{
	QString queryMessage = "SELECT `value` FROM device_param WHERE `name` = 'db_patch'";
	QScopedPointer<IDatabaseQuery> dbQuery(mDatabase.execQuery(queryMessage));
	if (!dbQuery)
	{
		LOG(mLog, LogLevel::Error, "Failed to check database patch version.");
		return 0;
	}

	if (dbQuery->first())
	{
		return dbQuery->value(0).toInt();
	}

	return 0;
}

//---------------------------------------------------------------------------
IDatabaseQuery * DatabaseUtils::prepareQuery(const QString & aQuery)
{
	QMutexLocker lock(&mAccessMutex);

	QScopedPointer<IDatabaseQuery> query(mDatabase.createQuery(aQuery));

	return query ? query.take() : nullptr;
}

//---------------------------------------------------------------------------
bool DatabaseUtils::execQuery(IDatabaseQuery * aQuery)
{
	QMutexLocker lock(&mAccessMutex);

	if (aQuery)
	{
		return aQuery->exec();
	}

	return false;
}

//---------------------------------------------------------------------------
void DatabaseUtils::releaseQuery(IDatabaseQuery * aQuery)
{
	QMutexLocker lock(&mAccessMutex);

	delete aQuery;
}

//---------------------------------------------------------------------------