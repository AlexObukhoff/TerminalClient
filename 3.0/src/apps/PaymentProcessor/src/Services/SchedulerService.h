/* @file Менеджер запуска задач по расписанию. */

#pragma once

// Boost
#include <boost/function.hpp>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QtCore/QTime>
#include <QtCore/QSettings>
#include <QtCore/QReadWriteLock>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>

// SDK
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ISchedulerService.h>
#include <SDK/PaymentProcessor/Core/ITask.h>

class IApplication;

/*
	Описание настроек ini файла

	Расположение: %WORK_DIR%/data/scheduler.ini

	[task_name]
	type   - имя класса выполняющего задачу
	period - интервал запуска задачи в секундах (используется, если не задано время запуска time)
	time   - время запуска задачи в формат ЧЧ:ММ или специальное значение startup/first_run
	repeat_count_if_fail - кол-во повторов запуска задачи, в случае ошибки выполнения задачи.
	time_threshold - кол-во секунд рандомизации времени запуска (в плюс)
	params - строка, передающаяся как параметр запускаемой задачи

	Расположение: %WORK_DIR%/user/scheduler_config.ini

	[task_name]
	last_execute - время последнего запуска задачи в формате ГГГГ.ММ.ДД ЧЧ:ММ:СС (этот параметр выставляется программой после запуска задачи)
*/

//---------------------------------------------------------------------------
class SchedulerService : 
	public QObject, 
	public SDK::PaymentProcessor::ISchedulerService, 
	public SDK::PaymentProcessor::IService, 
	private ILogable
{
	Q_OBJECT

public:
	/// Получение SchedulerService'а.
	static SchedulerService * instance(IApplication * aApplication);

	SchedulerService(IApplication * aApplication);
	virtual ~SchedulerService();

	/// IService: Инициализация сервиса.
	virtual bool initialize();

	/// IService: Закончена инициализация всех сервисов.
	virtual void finishInitialize();

	/// IService: Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	/// IService: Завершение работы сервиса.
	virtual bool shutdown();

	/// IService: Возвращает имя сервиса.
	virtual QString getName() const;

	/// IService: Список необходимых сервисов.
	virtual const QSet<QString> & getRequiredServices() const;

	/// IService: Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// IService: Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);

private:
	class Item
	{
	public:
		Item();
		Item(const QString & aName, const QSettings & aSettings, const QSettings & aUserSettings);

		/// Проверка корректности конфигурации задачи.
		bool isOK() const;

		/// Создать таймер сработающий в запланированное время.
		QTimer * createTimer();

		/// Запуск задачи.
		bool execute(SDK::PaymentProcessor::ITask * aTask, ILog * aLog);

		/// Записываем в item результат выполнения таска
		void complete(bool aComplete);

		QString name() const           { return mName; }
		QString type() const           { return mType; }
		QString params() const         { return mParams; }
		QDateTime lastExecute() const  { return mLastExecute; }
		bool onlyOnce() const          { return mOnlyOnce; }

		int failCount() const          { return mFailExecuteCounter; }

	private:
		QString mType;          // Тип задачи
		QString mParams;        // Параметры задачи
		QString mName;          // Имя задачи
		QTime   mTime;          // Время запуска задачи
		int mPeriod;            // Переодичность запуска задачи в секундах, действует если не задано конкретное время
		bool mTriggeredOnStart; // Запускать первый раз сразу при старте
		int mTimeThreshold;     // Максимальный разброс времени при запуске задачи в секундах
		int mRepeatCountIfFail; // Количество повторов в случае ошибки
		int mRetryTimeout;      // Таймаут повторного запуска в случае неуспеха в cекундах
		bool mOnlyOnce;         // Запускать задачу только один раз

	private:
		QDateTime mLastExecute;  // Время последнего запуска задачи
		int mFailExecuteCounter; // Счетчик неудачных запусков задачи
	};

private:
	/// Запустить таймер задания.
	bool schedule(Item & aItem) const;

	/// Сохранить время и результат последнего запуска задачи.
	void saveLastExecute(Item & aItem, bool aComplete);

	/// Включение автообновления клиента
	void setupAutoUpdate();

	/// Включение энергосберегающего режима дисплея по расписанию
	void setupDisplayOnOff();

private slots:
	/// Запустить таймеры всех заданий.
	void scheduleAll();

	/// Выполнить задание.
	void execute();

	/// Обработчик завершения выполнения таска
	void onTaskComplete(const QString & aName, bool aComplete);

private:
	IApplication * mApplication;
	QThread mThread;

	QMap<QString, Item> mItems;
	QMap<QString, SDK::PaymentProcessor::ITask *> mWorkingTasks;
	QReadWriteLock mLock;
};

//---------------------------------------------------------------------------
