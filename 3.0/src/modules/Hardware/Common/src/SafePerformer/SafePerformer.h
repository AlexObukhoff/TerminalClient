/* @file Класс для выполнения функционала без зависаний. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QThread>
#include <QtCore/QMetaType>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <Common/QtHeadersEnd.h>

// Common
#include <Common/ILog.h>

// Project
#include "Hardware/Common/FunctionTypes.h"

typedef std::function<void(const QString &, int, int)> TChangePerformingTimeout;

/// Результаты выполнения задачи.
namespace ETaskResult
{
	enum Enum
	{
		OK,          /// Задача выполнена, результат - ОК.
		Error,       /// Задача выполнена, результат - ошибка.
		Invalid,     /// Задача невыполнима (пустая).
		Suspended    /// Задача не выполнена (зависла).
	};
}

struct STaskData
{
	TBoolMethod task;
	TVoidMethod forwardingTask;
	TChangePerformingTimeout changePerformingTimeout;
	QString context;
	int timeout;
};

Q_DECLARE_METATYPE(STaskData);

//--------------------------------------------------------------------------------
/// Рабочий поток для класса-исполнителя.
class SafePerformerThread : public QThread
{
	Q_OBJECT

public:
	SafePerformerThread(ILog * aLog);

signals:
	/// Завершено.
	void finished(bool aSuccess);

public slots:
	/// Выполнить зависоноопасный функционал.
	void onTask(const STaskData & aData);

private:
	/// Лог.
	ILog * mLog;
};

//--------------------------------------------------------------------------------
/// Класс-исполнитель зависоноопасного функционала сторонних разработчиков. Синхронизация с логикой ТК не предусмотрена
class SafePerformer : public QObject
{
	Q_OBJECT

public:
	SafePerformer(ILog * aLog);

	/// Выполнение зависоно-опасной команды в другом потоке, с таймаутом.
	/// Таймаут должен исчерпывать все варианты выполнения команды с небольшим запасом.
	/// Если выполнение команды не укладывается в таймаут - считается, что команда зависла и поток не удалится никогда.
	ETaskResult::Enum process(const STaskData & aData);

public slots:
	/// Завершено.
	void onFinished(bool aSuccess);

private:
	bool mResult;                     /// Результат выполнения.
	QMutex mGuard;                    /// Сторож для wait condition.
	QWaitCondition mWaitCondition;    /// Wait condition для таймаута ожидания.

	/// Лог.
	ILog * mLog;
};

//--------------------------------------------------------------------------------
