/* @file Сервиса, владеющий клиентом БД. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILog.h>
#include <DatabaseProxy/IDatabaseProxy.h>
#include <DatabaseProxy/IDatabaseQuery.h>

// SDK
#include <SDK/PaymentProcessor/Core/ITerminalService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Project
#include "System/IApplication.h"

#include "DatabaseUtils/DatabaseUtils.h"

#include "Services/ServiceNames.h"
#include "Services/SettingsService.h"
#include "Services/DatabaseService.h"
#include "Services/EventService.h"

namespace PP = SDK::PaymentProcessor;

namespace CDatabaseService
{
	/// Максимальное кол-во ошибок базы, после которых терминал блокирует свою работу
	const int MaximumDatabaseErrors = 13;
}

//---------------------------------------------------------------------------
DatabaseService * DatabaseService::instance(IApplication * aApplication)
{
	return static_cast<DatabaseService *>(aApplication->getCore()->getService(CServices::DatabaseService));
}

//---------------------------------------------------------------------------
DatabaseService::DatabaseService(IApplication * aApplication)
	: mApplication(aApplication),
	  mDatabase(0),
	  mErrorCounter(0)
{
}

//---------------------------------------------------------------------------
DatabaseService::~DatabaseService()
{
	if (mDatabase)
	{
		IDatabaseProxy::freeInstance(mDatabase);
		mDatabase = 0;
	}
}

//---------------------------------------------------------------------------
bool DatabaseService::initialize()
{
	LOG(mApplication->getLog(), LogLevel::Normal, "Initializing database...");

	PP::TerminalSettings * terminalSettings = SettingsService::instance(mApplication)->getAdapter<PP::TerminalSettings>();

	// Инициализация базы данных.
	mDatabase = IDatabaseProxy::getInstance(this);
	if (!mDatabase)
	{
		LOG(mApplication->getLog(), LogLevel::Error, "Can't get database proxy instance.");
		return false;
	}

	mDbUtils = QSharedPointer<DatabaseUtils>(new DatabaseUtils(*mDatabase, mApplication));

	SDK::PaymentProcessor::SDatabaseSettings dbSettings = terminalSettings->getDatabaseSettings();

	LOG(mApplication->getLog(), LogLevel::Normal, QString("Opening terminal database (host: %1, port: %2, name: %3)...")
		.arg(dbSettings.host)
		.arg(dbSettings.port)
		.arg(dbSettings.name));

	bool integrityFailed = false;
	QStringList errorsList;

	try
	{
		for (int retryCount = 0; retryCount < 2; ++retryCount)
		{
			// Подключаемся к БД.
			if (!mDatabase->open(dbSettings.name, dbSettings.user, dbSettings.password, dbSettings.host, dbSettings.port))
			{
				throw QString("cannot open database");
			}

			errorsList.clear();

			// Проверка на ошибку полностью испорченного формата базы
			integrityFailed = !mDatabase->checkIntegrity(errorsList) ||
				errorsList.filter(QRegExp("*malformed*", Qt::CaseInsensitive, QRegExp::Wildcard)).size() ||
				!mDbUtils->initialize();

			if (integrityFailed)
			{
				LOG(mApplication->getLog(), LogLevel::Error, "Failed check database integrity. Backup broken database and create new.");

				// Закрываем БД и переименовываем файл базы
				QString databaseFile = mDatabase->getCurrentBaseName();
				mDatabase->close();
				QFile::rename(databaseFile, databaseFile + QString(".backup_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz")));

				// Выставить ошибочный статус устройства "терминал"
				EventService::instance(mApplication)->sendEvent(
					SDK::PaymentProcessor::Event(SDK::PaymentProcessor::EEventType::Critical, getName(), "Database integrity check failed"));

				continue;
			}

			if (retryCount && !integrityFailed)
			{
				// Отмечаем статус устройста, что БД была восстановлена
				EventService::instance(mApplication)->sendEvent(
					SDK::PaymentProcessor::Event(SDK::PaymentProcessor::EEventType::OK, getName(), "New database was created."));
			}

			//TODO - Запускаем процедуру восстановления базы
			// http://blog.niklasottosson.com/?p=852
			break;
		}
	}
	catch (QString & error)
	{
		LOG(mApplication->getLog(), LogLevel::Error, QString("Failed to initialize database manager: %1.").arg(error));
		return false;
	}

	return !integrityFailed;
}

//------------------------------------------------------------------------------
void DatabaseService::finishInitialize()
{
}

//---------------------------------------------------------------------------
bool DatabaseService::canShutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool DatabaseService::shutdown()
{
	mDbUtils.clear();

	if (mDatabase)
	{
		IDatabaseProxy::freeInstance(mDatabase);
		mDatabase = 0;
	}

	return true;
}

//---------------------------------------------------------------------------
QString DatabaseService::getName() const
{
	return CServices::DatabaseService;
}

//---------------------------------------------------------------------------
const QSet<QString> & DatabaseService::getRequiredServices() const
{
	static QSet<QString> requiredServices = QSet<QString>()
		<< CServices::SettingsService
		<< CServices::EventService;

	return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap DatabaseService::getParameters() const
{
	return QVariantMap();
}

//---------------------------------------------------------------------------
void DatabaseService::resetParameters(const QSet<QString> &)
{
}

/// Выполнение запроса по строке.
bool DatabaseService::execQuery(const QString & aQuery)
{
	auto query = prepareQuery(aQuery);

	return execQuery(query);
}

//---------------------------------------------------------------------------
QSharedPointer<IDatabaseQuery> DatabaseService::prepareQuery(const QString & aQuery)
{
	auto queryDeleter = [&](IDatabaseQuery * aQuery)
	{
		mDbUtils->releaseQuery(aQuery);
	};
	
	return QSharedPointer<IDatabaseQuery>(mDbUtils->prepareQuery(aQuery), queryDeleter);
}

//---------------------------------------------------------------------------
bool DatabaseService::execQuery(QSharedPointer<IDatabaseQuery> aQuery)
{
	return mDbUtils->execQuery(aQuery.data());
}

//---------------------------------------------------------------------------
QSharedPointer<IDatabaseQuery> DatabaseService::createAndExecQuery(const QString & aQuery)
{
	auto query = prepareQuery(aQuery);

	if (execQuery(query))
	{
		return query;
	}

	return QSharedPointer<IDatabaseQuery>();
}

//---------------------------------------------------------------------------
bool DatabaseService::isGood(bool aQueryResult)
{
	if (!aQueryResult && ++mErrorCounter >= CDatabaseService::MaximumDatabaseErrors)
	{
		const char message[] = "Database error counter has reached a limit value. Lock the terminal due to a DB error.";
		LOG(mApplication->getLog(), LogLevel::Error, message);

		SDK::PaymentProcessor::Event e(SDK::PaymentProcessor::EEventType::TerminalLock, "DatabaseService");
		mApplication->getCore()->getEventService()->sendEvent(e);

		static bool feedbackSent = false;

		if (!feedbackSent)
		{
			mApplication->getCore()->getTerminalService()->sendFeedback(CServices::DatabaseService, message);

			feedbackSent = true;
		}
	}

	return aQueryResult;
}

//---------------------------------------------------------------------------