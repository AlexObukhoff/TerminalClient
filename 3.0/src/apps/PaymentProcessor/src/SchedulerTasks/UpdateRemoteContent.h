/* @file Реализация задачи обновления контента удалённых сервисов. */

#pragma once

// SDK
#include <SDK/PaymentProcessor/Core/ITask.h>

// Модули
#include <Common/ILogable.h>

//---------------------------------------------------------------------------
class UpdateRemoteContent : public QObject, public SDK::PaymentProcessor::ITask
{
	Q_OBJECT

public:
	UpdateRemoteContent(const QString & aName, const QString & aLogName, const QString & aParams);

	/// выполнить задачу
	virtual void execute();

	/// остановить выполнение задачи
	virtual bool cancel() { return true; };

	/// подписаться на сигнал окончания задания
	virtual bool subscribeOnComplete(QObject * aReceiver, const char * aSlot);

signals:
	void finished(const QString & aName, bool aComplete);
};


