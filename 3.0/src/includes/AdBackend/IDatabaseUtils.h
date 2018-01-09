/* @file Интерфейс базы данных. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <Common/QtHeadersEnd.h>

// Project
#include "StatisticRecord.h"

namespace Ad
{

//------------------------------------------------------------------------
/// Интерфейс базы данных.
class IDatabaseUtils
{
public:
	/// Инкрементировать значение счетчика показов канала
	virtual bool addStatisticRecord(qint64 aId, const QString & aChannel) = 0;

	/// Установить значение счетчика показов канала
	virtual bool setStatisticRecord(qint64 aId, const QString & aChannel, int aValue) = 0;

	/// Заполняет список aRecords неотправленными на сервер записями рекламной статистики.
	/// Полем aLimit можно ограничить кол-во получаемых записей (aLimit = -1, ограничений нет).
	virtual bool getUnsentStatisticRecords(QList<SStatisticRecord> & aRecords, int aLimit = -1) = 0;

	/// Удаляет из базы записи, указанные в aRecords.
	virtual bool deleteStatisticRecords(const QList<SStatisticRecord> & aRecords) = 0;

protected:
	virtual ~IDatabaseUtils() {}
};

//------------------------------------------------------------------------
} // namespace Ad
