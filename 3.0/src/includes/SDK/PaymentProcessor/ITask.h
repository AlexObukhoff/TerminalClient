/* @file Интерфейс задачи для менеджера задач. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <Common/QtHeadersEnd.h>

//------------------------------------------------------------------------------
namespace SDK
{

//------------------------------------------------------------------------------
namespace PaymentProcessor
{

namespace TaskContext
{
	const int LastActivation = 0;
	const int CurrentTimestamp = 1;
	
	const int UserProperty   = 100;
}

/// Интерфейс задачи для планировщика.
class ITask
{
public:
	typedef QMap<int, QVariant> TContext;

	virtual ~ITask() {};

	/// Предикат возвращает true, если задача может быть выполнена в данный момент.
	virtual bool isReady(TContext & aContext) = 0;

	/// Возвращает true, если задача должна выполняться в отдельном потоке.
	virtual bool isThread() const = 0;

	/// Рабочая процедура задачи.
	virtual void run() = 0;
};

//------------------------------------------------------------------------------
} // PaymentProcessor

//------------------------------------------------------------------------------
} // SDK

//------------------------------------------------------------------------------

