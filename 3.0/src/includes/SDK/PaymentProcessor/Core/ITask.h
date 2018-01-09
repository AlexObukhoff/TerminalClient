/* @file Интерфейс сервиса, обеспечивающего запуск задач в определенное время. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// Модули
#include <Common/ILog.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class ITask
{
protected:
	QString mName;

public:
	ITask(const QString & aName, const QString & /*aLogName*/, const QString & /*aParams*/) : mName(aName) {}
	virtual ~ITask() {}

	/// выполнить задачу
	virtual void execute() = 0;

	/// остановить выполнение задачи
	virtual bool cancel() = 0;

	/// подписаться на сигнал окончания задания
	virtual bool subscribeOnComplete(QObject * aReceiver, const char * aSlot) = 0;
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor
