/* @file Реализация базы данных. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtSql/QSqlDatabase>
#include <Common/QtHeadersEnd.h>

// Common
#include <Common/ILogable.h>
#include <AdBackend/IDatabaseUtils.h>

//------------------------------------------------------------------------
namespace Ad
{

namespace CDatabaseUtils
{
	const QString LogName = "Ad";
}

//------------------------------------------------------------------------
/// Реализация базы данных.
class DatabaseUtils : public IDatabaseUtils, public ILogable
{
public:
	DatabaseUtils(const QString & aWorkingDirectory, ILog * aLog);
	virtual ~DatabaseUtils();

	/// Инкрементировать значение счетчика показов канала
	virtual bool addStatisticRecord(qint64 aId, const QString & aChannel);

	/// Установить значение счетчика показов канала
	virtual bool setStatisticRecord(qint64 aId, const QString & aChannel, int aValue);

	/// IDatabaseUtils: Заполняет список aRecords неотправленными на сервер записями рекламной статистики.
	/// Полем aLimit можно ограничить кол-во получаемых записей (aLimit = -1, ограничений нет).
	virtual bool getUnsentStatisticRecords(QList<SStatisticRecord> & aRecords, int aLimit = -1);

	/// IDatabaseUtils: Удаляет из базы записи, указанные в aRecords.
	virtual bool deleteStatisticRecords(const QList<SStatisticRecord> & aRecords);

private:
	QSqlDatabase mDatabase;
};

//------------------------------------------------------------------------

} // namespace Ad
