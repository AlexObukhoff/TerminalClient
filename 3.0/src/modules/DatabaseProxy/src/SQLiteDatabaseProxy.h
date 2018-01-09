#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtSql/QtSql>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>

#include <DatabaseProxy/IDatabaseProxy.h>
#include <DatabaseProxy/IDatabaseQuery.h>

class QSqlDatabase;

//---------------------------------------------------------------------------
namespace CSQLiteDatabaseProxy
{
	const QString DriverName  = "QSQLITE";
	const int     BusyTimeout = 5000; // ms
}

//---------------------------------------------------------------------------
class SQLiteDatabaseProxy : public IDatabaseProxy, protected ILogable
{
	friend class IDatabaseProxy;

public:
	SQLiteDatabaseProxy();

	virtual ~SQLiteDatabaseProxy();

	/// Установить интерфейс контроля над ошибками БД
	virtual void setQueryChecker(IDatabaseQueryChecker * aQueryChecker);

	/// IDatabaseProxy: Открытие соединения с БД.
	virtual bool open(const QString & aDatabaseName = CIDatabaseProxy::DefaultDatabase,
	                  const QString & aUser         = CIDatabaseProxy::DefaultUser,
	                  const QString & aPassword     = CIDatabaseProxy::DefaultPassword,
	                  const QString & aHost         = CIDatabaseProxy::DefaultHost,
	                  const int       aPort         = CIDatabaseProxy::DefaultPort);

	/// IDatabaseProxy: Закрытие соединения с БД.
	virtual void close();

	/// IDatabaseProxy: Возвращает true, если база открыта.
	virtual bool isConnected() const;

	/// IDatabaseProxy: Возвращает имя открытой БД.
	virtual const QString & getCurrentBaseName() const;

	/// IDatabaseProxy: Создаёт экземпляр запроса к БД.
	virtual IDatabaseQuery* createQuery();

	/// Создает и подготавливает экземляр запроса к БД.
	virtual IDatabaseQuery * createQuery(const QString & aQueryString);

	/// IDatabaseProxy: Выполнение DML запроса. Помещает в rowsAffected количество затронутых строк.
	virtual bool execDML(const QString & aQuery, long & aRowsAffected);

	/// IDatabaseProxy: Выполнение запроса, содержащего, к примеру, COUNT(*). В result записывает значение ячейки (1,1).
	virtual bool execScalar(const QString & aQuery, long & aResult);

	/// IDatabaseProxy: Выполнение произвольного запроса. Если запрос успешно выполнен, то результат функции будет не нулевым.
	virtual IDatabaseQuery * execQuery(const QString & aQuery);

	/// IDatabaseProxy: Пытается создать новую транзакцию.
	virtual bool transaction();

	/// IDatabaseProxy: Принимает изменения, внесённые во время последней транзакции.
	virtual bool commit();

	/// IDatabaseProxy: Сбрасывает изменения, внесённые во время последней транзакции.
	virtual bool rollback();

public:
	/// Проверка целостности базы
	virtual bool checkIntegrity(QStringList & aListErrors);

protected:
	virtual bool safeExec(QSqlQuery * aQuery, const QString & aQueryMessage);

private:
	QSharedPointer<QSqlDatabase> mDb;
	QMutex                      mMutex;
	QString                     mCurrentBase;
	IDatabaseQueryChecker *     mQueryChecker;
};

//---------------------------------------------------------------------------
