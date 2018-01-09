/* @file Абстрактный провайдер СУБД. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

class IDatabaseQuery;

//---------------------------------------------------------------------------
namespace CIDatabaseProxy
{
	const QString DefaultDatabase   = "tc.db";
	const QString DefaultHost       = "";
	const QString DefaultUser       = "";
	const QString DefaultPassword   = "";
	const int     DefaultPort       = 0;
	const QString DateFormat        = "yyyy-MM-dd hh:mm:ss.zzz";
	const QString PaymentDateFormat = "yyyy-MM-dd hh:mm:ss";
	const QString ShortDateFormat   = "yyyy-MM-dd 00:00:00";
	const QString LogName           = "DatabaseProxy";
}

//---------------------------------------------------------------------------
class IDatabaseQueryChecker
{
public:
	/// Проверка результата работы запроса к БД
	virtual bool isGood(bool aQueryResult) = 0;
};

//---------------------------------------------------------------------------
class IDatabaseProxy
{
public:
	/// Тип базы данных.
	enum Type
	{
		SQLite = 0,
		MySql
	};

	/// Вызываем этот метод для получения текущей реализации интерфейса БД.
	static IDatabaseProxy * getInstance(IDatabaseQueryChecker * aErrorChecker, Type aType = SQLite);

	/// Освобождает память, занимаемую созданным экземпляром интерфейса БД.
	static void freeInstance(IDatabaseProxy * aProxy);

	/// Установить интерфейс контроля над ошибками БД
	virtual void setQueryChecker(IDatabaseQueryChecker * aQueryChecker) = 0;

	/// Открытие соединения с БД.
	virtual bool open(const QString & aDatabaseName = CIDatabaseProxy::DefaultDatabase,
	                  const QString & aUser         = CIDatabaseProxy::DefaultUser,
					  const QString & aPassword     = CIDatabaseProxy::DefaultPassword,
					  const QString & aHost         = CIDatabaseProxy::DefaultHost,
					  const int       aPort         = CIDatabaseProxy::DefaultPort) = 0;

	/// Закрытие соединения с БД.
	virtual void close() = 0;

	/// Возвращает true, если база открыта.
	virtual bool isConnected() const = 0;

	/// Возвращает имя открытой БД.
	virtual const QString& getCurrentBaseName() const = 0;

	/// Создаёт экземпляр запроса к БД.
	virtual IDatabaseQuery * createQuery() = 0;

	/// Создает и подготавливает экземляр запроса к БД.
	virtual IDatabaseQuery * createQuery(const QString & aQueryString) = 0;

	/// Выполнение DML запроса. Помещает в rowsAffected количество затронутых строк.
	virtual bool execDML(const QString & aQuery, long & aRowsAffected) = 0;

	/// Выполнение запроса, содержащего, к примеру, COUNT(*). В result записывает значение ячейки (1,1).
	virtual bool execScalar(const QString & aQuery, long & aResult) = 0;

	/// Выполнение произвольного запроса. Если запрос успешно выполнен, то результат функции будет не нулевым.
	virtual IDatabaseQuery* execQuery(const QString & aQuery) = 0;

	/// Пытается создать новую транзакцию.
	virtual bool transaction() = 0;

	/// Принимает изменения, внесённые во время последней транзакции.
	virtual bool commit() = 0;

	/// Сбрасывает изменения, внесённые во время последней транзакции.
	virtual bool rollback() = 0;

public:
	/// Проверка целостности базы
	virtual bool checkIntegrity(QStringList & aListErrors) = 0;

protected:
	virtual ~IDatabaseProxy() {};
};

//---------------------------------------------------------------------------
