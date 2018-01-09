/* @file Интерфейс сервиса для работы с БД. */

#pragma once

// Qt headers
#include "Common/QtHeadersBegin.h"
#include <QtCore/QSharedPointer>
#include "Common/QtHeadersEnd.h"

class IDatabaseQuery;

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class IDatabaseService
{
public:
	/// Выполнение запроса по строке.
	virtual bool execQuery(const QString & aQuery) = 0;

	/// Подготавливает запрос к биндингу параметров.
	virtual QSharedPointer<IDatabaseQuery> prepareQuery(const QString & aQuery) = 0;

	/// Создание запроса по строке и его выполнение.
	virtual QSharedPointer<IDatabaseQuery> createAndExecQuery(const QString & aQuery) = 0;

	/// Выполнение переданного запроса.
	virtual bool execQuery(QSharedPointer<IDatabaseQuery> aQuery) = 0;

protected:
	virtual ~IDatabaseService() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor
