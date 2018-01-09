#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QMutex>
#include <QtSql/QtSql>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILog.h>

#include <DatabaseProxy/IDatabaseProxy.h>
#include <DatabaseProxy/IDatabaseQuery.h>

class QSqlDatabase;
class IPayment;

//---------------------------------------------------------------------------
namespace CMySqlDatabaseProxy
{
	const QString DriverName = "QMYSQL";
	const QString DefaultLog = "MySql";
	const QString DateFormat = "yyyy-MM-dd hh:mm:ss";
	const QString DefaultUser = "root";
	const QString DefaultPass = "";
	const QString DefaultHost = "localhost";
	const int DefaultPort = 3306;
	const QString ConnactionName = "DefaultConnection";
}

//---------------------------------------------------------------------------
class MySqlDatabaseProxy : public IDatabaseProxy
{
	friend class IDatabaseProxy;

public:
	MySqlDatabaseProxy();
	~MySqlDatabaseProxy();

	/// Установить интерфейс контроля над ошибками БД
	virtual void setQueryChecker(IDatabaseQueryChecker * aQueryChecker);

	virtual bool open(const QString& aDatabaseName, 
	                  const QString& aUser = CMySqlDatabaseProxy::DefaultUser, 
					  const QString& aPassword = CMySqlDatabaseProxy::DefaultPass, 
					  const QString& aHost = CMySqlDatabaseProxy::DefaultHost,
	                  const int aPort = CMySqlDatabaseProxy::DefaultPort);
	virtual void close();

	virtual bool isConnected() const;
	virtual const QString& getCurrentBaseName() const;

	// Создаёт экземпляр запроса к БД.
	virtual IDatabaseQuery* createQuery();

	/// Создает и подготавливает экземляр запроса к БД.
	virtual IDatabaseQuery * createQuery(const QString & aQueryString);

	/*!< Выполнение DML запроса. Помещает в rowsAffected количество затронутых строк. */
	virtual bool execDML(const QString& strQuery, long& rowsAffected);
	/*!< Выполнение запроса, содержащего, к примеру, COUNT(*). В result записывает значение ячейки (1,1). */
	virtual bool execScalar(const QString& strQuery, long& result);
	/*!< Выполнение произвольного запроса. Если запрос успешно выполнен, то результат функции будет не нулевым. */
	virtual IDatabaseQuery* execQuery(const QString& strQuery);

	/*!< Пытается создать новую транзакцию. */
	virtual bool transaction();
	/*!< Принимает изменения, внесённые во время последней транзакции. */
	virtual bool commit();
	/*!< Сбрасывает изменения, внесённые во время последней транзакции. */
	virtual bool rollback();

public:
	/// Проверка целостности базы
	virtual bool checkIntegrity(QStringList & aListErrors);

protected:
	virtual bool safeExec(QSqlQuery* query, const QString& queryMessage);

private:
	QMutex         mMutex;
	QSqlDatabase * mDb;
	QString        mCurrentBase;
	ILog *         mLog;
	IDatabaseQueryChecker * mQueryChecker;
};

//---------------------------------------------------------------------------
