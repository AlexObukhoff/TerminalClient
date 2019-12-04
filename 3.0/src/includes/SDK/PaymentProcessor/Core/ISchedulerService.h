/* @file Интерфейс сервиса, обеспечивающего запуск задач в определенное время. */

#pragma once

#include <boost/function.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// Модули
#include <Common/ILog.h>
#include <SDK/PaymentProcessor/Core/ITask.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class ISchedulerService
{
protected:
	virtual ~ISchedulerService() {}

public:
	typedef boost::function<SDK::PaymentProcessor::ITask *(const QString & aName, const QString & aLogName, const QString & aParams)> TTaskCreator;

	/// зарегистрировать тип задачи в фабрике классов
	template <class C>
	void registerTaskType(const QString & aType)
	{
		auto taskCreator = [](const QString & aName, const QString & aLogName, const QString & aParams) -> SDK::PaymentProcessor::ITask *
		{
			return new C(aName, aLogName, aParams);
		};

		if (!mFactory.contains(aType))
		{
			mFactory[aType] = TTaskCreator(taskCreator);
		}
	}

	void registerTaskType(const QString & aName, SDK::PaymentProcessor::ITask * aTask)
	{
		if (!mExternalTasks.contains(aName))
		{
			mExternalTasks[aName] = aTask;
		}		
	}

protected:
	QMap<QString, TTaskCreator> mFactory;
	QMap<QString, SDK::PaymentProcessor::ITask *> mExternalTasks;
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor
