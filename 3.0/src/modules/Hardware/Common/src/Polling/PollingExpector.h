/* @file Класс-expector для ожидания состояния. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <Common/QtHeadersEnd.h>

// Project
#include "Hardware/Common/FunctionTypes.h"
#include "Hardware/Common/WaitingData.h"

//--------------------------------------------------------------------------------
/// Рабочий поток для класса-ожидателя.
class ExpectorWorkingThread : public QThread
{
	Q_OBJECT

public:
	ExpectorWorkingThread();
	void process(TBoolMethod aOnPoll, TBoolMethod aCondition, TBoolMethod aErrorCondition, int aPollingInterval, bool aPollingSensible);

public slots:
	void onPoll();    /// Опрос состояния.

signals:
	void finished(bool aSuccess);    /// Завершено.

private:
	virtual void run();              /// QThread: Рабочая процедура Qt'шной нити.

	QThread * mOwner;                /// Указатель на вызвавший поток.
	QTimer mPolling;                 /// Таймер для поллинга.
	TBoolMethod mOnPoll;             /// Функтор поллинга.
	TBoolMethod mCondition;          /// Функтор условия ожидания.
	TBoolMethod mErrorCondition;     /// Функтор условия ошибки.
	bool mPollingSensible;           /// В условие ожидания включен контроль результата поллинга.
};

//--------------------------------------------------------------------------------
/// Класс-ожидатель.
class PollingExpector : public QObject
{
	Q_OBJECT

public:
	PollingExpector();

	/// Ожидание состояния или выполнения полла.
	bool wait(TBoolMethod aCondition, int aPollingInterval, int aTimeout, bool aPollingSensible = false);
	bool wait(TBoolMethod aCondition, const SWaitingData & aWaitingData);

	/// Ожидание состояния.
	template <class T>
	bool wait(std::function<T()> aOnPoll, TBoolMethod aCondition, int aPollingInterval, int aTimeout, bool aPollingSensible = false);
	template <class T>
	bool wait(std::function<T()> aOnPoll, TBoolMethod aCondition, const SWaitingData & aWaitingData);

	/// Ожидание состояния или ошибки.
	template <class T>
	bool wait(std::function<T()> aOnPoll, TBoolMethod aCondition, TBoolMethod aErrorCondition, int aPollingInterval, int aTimeout, bool aPollingSensible = false);
	template <class T>
	bool wait(std::function<T()> aOnPoll, TBoolMethod aCondition, TBoolMethod aErrorCondition, const SWaitingData & aWaitingData);

public slots:
	void onFinished(bool aSuccess);          /// Завершено.

private:
	bool mResult;                            /// Результат ожидания.
	QMutex mGuard;                           /// Сторож для wait condition.
	QWaitCondition mWaitCondition;           /// Wait condition для таймаута ожидания.
	ExpectorWorkingThread mWorkingThread;    /// Рабочий поток.
};

//--------------------------------------------------------------------------------
