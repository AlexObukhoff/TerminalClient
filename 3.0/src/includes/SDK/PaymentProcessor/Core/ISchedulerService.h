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
class ISchedulerService
{
protected:
	virtual ~ISchedulerService() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor
