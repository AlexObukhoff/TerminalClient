/* @file Реализация задачи включения энергосберегающего режима. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTime>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ITask.h>

// Модули
#include <Common/ILogable.h>

//---------------------------------------------------------------------------
class OnOffDisplay : public QObject, public SDK::PaymentProcessor::ITask, public ILogable
{
	Q_OBJECT

public:
	OnOffDisplay(const QString & aName, const QString & aLogName, const QString & aParams);
	virtual ~OnOffDisplay();

	/// выполнить задачу
	virtual void execute();

	/// остановить выполнение задачи
	virtual bool cancel() { return true; }

	/// подписаться на сигнал окончания задания
	virtual bool subscribeOnComplete(QObject * aReceiver, const char * aSlot);

protected:
	bool mEnable;
	QTime mFrom;
	QTime mTill;
	enum
	{
		Standby,
		ScreenSaver,
		Shutdown
	} mType;

signals:
	void finished(const QString & aName, bool aComplete);
};

