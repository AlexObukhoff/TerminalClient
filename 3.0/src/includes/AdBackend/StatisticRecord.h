/* @file Структура, содержащая информацию о показе рекламного баннера. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

namespace Ad
{

//------------------------------------------------------------------------
struct SStatisticRecord
{
	SStatisticRecord()
	{
		recordId = -1;
		id = 0;
		duration = 0;
	}

	SStatisticRecord(qint64 aRecordId, qint64 aId, const QString & aChannel, const QDate & aDate, int aDuration) :
		recordId(aRecordId), id(aId), channel(aChannel), date(aDate), duration(aDuration)
	{
	}

	/// Идентификатор записи в базе данных.
	qint64 recordId;
	/// Идентификатор рекламного баннера.
	qint64 id;
	/// Канал рекламного баннера.
	QString channel;
	/// Дата события.
	QDate date;
	/// Кол-во событий показа.
	int duration;
};

//------------------------------------------------------------------------
} // namespace Ad
