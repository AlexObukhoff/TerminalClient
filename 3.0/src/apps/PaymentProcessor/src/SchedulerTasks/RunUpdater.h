/* @file Реализация задачи запуска модуля обновления. */

#pragma once

// SDK
#include <SDK/PaymentProcessor/Core/ITask.h>

// Модули
#include <Common/ILogable.h>

//---------------------------------------------------------------------------
class RunUpdater : public QObject, public SDK::PaymentProcessor::ITask, public ILogable
{
	Q_OBJECT

public:
	RunUpdater(const QString & aName, const QString & aLogName, const QString & aParams);
	virtual ~RunUpdater();

	/// выполнить задачу
	virtual void execute();

	/// остановить выполнение задачи
	virtual bool cancel() { return true; }

	/// подписаться на сигнал окончания задания
	virtual bool subscribeOnComplete(QObject * aReceiver, const char * aSlot);

signals:
	void finished(const QString & aName, bool aComplete);

protected:
	QString mParams;
};

