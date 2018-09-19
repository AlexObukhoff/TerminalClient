/* @file Менеджер запуcка задач по раcпиcанию. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <QtCore/QDir>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>

// Модули
#include <System/IApplication.h>
#include <System/SettingsConstants.h>
#include <Services/ServiceNames.h>
#include <Services/ServiceCommon.h>

// Проект
#include <Services/SchedulerService.h>
#include "SchedulerTasks/LogArchiver.h"
#include "SchedulerTasks/LogRotate.h"
#include "SchedulerTasks/RunUpdater.h"
#include "SchedulerTasks/TimeSync.h"
#include "SchedulerTasks/OnOffDisplay.h"
#include "SchedulerTasks/UpdateRemoteContent.h"

namespace PPSDK = SDK::PaymentProcessor;

namespace CScheduler
{
	const QString ConfigName = "/data/scheduler.ini";
	const QString UserConfigName = "/user/scheduler_config.ini";
	const QString ThreadName = "SchedulerThread";
	const QString LogName = "Scheduler";
	const QString TimeFormat = "hh:mm";
	const QString DateTimeFormat = "yyyy.MM.dd hh:mm:ss";

	const int StartTimeIfExpired = 10 * 60; // таймаут в cекундах для cлучая, еcли мы пропуcтили запуcк задачи

	namespace Config
	{
		const QString Type = "type";
		const QString Params = "params";
		const QString Time = "time";     // конкретное время запуcка задачи
		const QString Period = "period"; // Еcли time не задано, то берётcя период запуcка в cекундах
		const QString TriggeredOnStart = "triggered_on_start"; // Запускать первый раз сразу при старте
		const QString RepeatCountIfFail = "repeat_count_if_fail";
		const QString TimeThreshold = "time_threshold"; // максимальный порог времени, добавляемый рандомно к времени запуска задачи
		const QString RetryTimeout = "retry_timeout"; // таймаут повторного запуска в случае неуспеха в cекундах
		const QString OnlyOnce = "only_once";   // запускать задачу только один раз

		const QString StartupTime = "startup";     // запускать после каждого старта ПО
		const QString AfterFirstRun = "first_run"; // задача запускающаяся 1 раз после первой установки
	}

	namespace UserConfig
	{
		const QString LastExecute = "last_execute";
		const QString FailExecuteCounter = "fail_execute_counter";
	}
	
	const QString AutoUpdateTaskName = "AutoUpdate";
	const QString DisplayOnOffTaskName = "DisplayOnOff";
}

//---------------------------------------------------------------------------
SchedulerService * SchedulerService::instance(IApplication * aApplication)
{
	return static_cast<SchedulerService *>(aApplication->getCore()->getService(CServices::SchedulerService));
}

//---------------------------------------------------------------------------
SchedulerService::SchedulerService(IApplication * aApplication) :
	mApplication(aApplication),
	ILogable(CScheduler::LogName)
{
	mThread.setObjectName(CScheduler::ThreadName);

	moveToThread(&mThread);

	registerTaskType<LogArchiver>("LogArchiver");
	registerTaskType<LogRotate>("LogRotate");
	registerTaskType<RunUpdater>("RunUpdater");
	registerTaskType<TimeSync>("TimeSync");
	registerTaskType<OnOffDisplay>("OnOffDisplay");
	registerTaskType<UpdateRemoteContent>("UpdateRemoteContent");

	// для запуcка таймеров вcех заданий
	connect(&mThread, SIGNAL(started()), this, SLOT(scheduleAll()));
}

//---------------------------------------------------------------------------
SchedulerService::~SchedulerService()
{
}

//---------------------------------------------------------------------------
bool SchedulerService::initialize()
{
	QSettings settings(IApplication::toAbsolutePath(IApplication::getWorkingDirectory() + CScheduler::ConfigName), QSettings::IniFormat);
	QSettings userSettings(IApplication::toAbsolutePath(IApplication::getWorkingDirectory() + CScheduler::UserConfigName), QSettings::IniFormat);

	foreach (QString taskName, settings.childGroups())
	{
		settings.beginGroup(taskName);
		userSettings.beginGroup(taskName);

		Item item(taskName, settings, userSettings);

		if (!mFactory.contains(item.type()))
		{
			toLog(LogLevel::Error, QString("[%1]: Unknown task type '%2'.").arg(item.name()).arg(item.type()));
		}
		else if (!item.isOK())
		{
			toLog(LogLevel::Error, QString("[%1]: Invalid configuration. Skipped.").arg(taskName));
		}
		else
		{
			mItems.insert(item.name(), item);
			toLog(LogLevel::Normal, QString("[%1]: Loaded.").arg(taskName));
		}
		settings.endGroup();
		userSettings.endGroup();
	}

	setupAutoUpdate();
	setupDisplayOnOff();

	toLog(LogLevel::Normal, "Scheduler service initialized.");

	QTime time(QTime::currentTime());
	qsrand(unsigned(time.hour() + time.minute() + time.second() + time.msec()));

	return true;
}

//---------------------------------------------------------------------------
void SchedulerService::setupAutoUpdate()
{
	PPSDK::TerminalSettings * terminalSettings = static_cast<PPSDK::TerminalSettings *>(mApplication->getCore()->getSettingsService()->
		getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

	QTime startTime = terminalSettings->autoUpdate();

	if (startTime.isValid() && !startTime.isNull())
	{
		toLog(LogLevel::Normal, QString("Enabled auto update at '%1'.").arg(startTime.toString(CScheduler::TimeFormat)));

		// удаляем все задачи обновления
		foreach (auto task, mItems.values())
		{
			if (task.type() == "RunUpdater")
			{
				mItems.remove(task.name());
				toLog(LogLevel::Normal, QString("Removed task [%1].").arg(task.name()));
			}
		}

		QSettings settings;
		settings.beginGroup(CScheduler::AutoUpdateTaskName);
		settings.setValue(CScheduler::Config::Type, "RunUpdater");
		settings.setValue(CScheduler::Config::Time, startTime.toString(CScheduler::TimeFormat));
		// что бы терминалы с одинаковыми настройками не ломанулись обновляться все одновременно (анти DDOS)
		settings.setValue(CScheduler::Config::TimeThreshold, 3600);
		settings.setValue(CScheduler::Config::RepeatCountIfFail, 2);
		settings.setValue(CScheduler::Config::RetryTimeout, 3600);

		QSettings userSettings(IApplication::toAbsolutePath(IApplication::getWorkingDirectory() + CScheduler::UserConfigName), QSettings::IniFormat);
		userSettings.beginGroup(CScheduler::AutoUpdateTaskName);

		Item item(CScheduler::AutoUpdateTaskName, settings, userSettings);
		mItems.insert(item.name(), item);
		toLog(LogLevel::Normal, QString("[%1]: Loaded.").arg(item.name()));
	}
}

//------------------------------------------------------------------------------
SchedulerService::Item::Item() :
	mPeriod(0),
	mTriggeredOnStart(false),
	mTimeThreshold(0),
	mRepeatCountIfFail(0),
	mRetryTimeout(0),
	mOnlyOnce(false),
	mFailExecuteCounter(0)
{
}

//------------------------------------------------------------------------------
SchedulerService::Item::Item(const QString & aName, const QSettings & aSettings, const QSettings & aUserSettings) :
	mName(aName),
	mFailExecuteCounter(0),
	mOnlyOnce(false)
{
	mType = aSettings.value(CScheduler::Config::Type).toString();
	
	QVariant params = aSettings.value(CScheduler::Config::Params);
	mParams = params.type() == QVariant::StringList ? params.toStringList().join(",") : params.toString();

	mPeriod = aSettings.value(CScheduler::Config::Period, "0").toInt();
	mTriggeredOnStart = aSettings.value(CScheduler::Config::TriggeredOnStart, "false").toString().compare("true", Qt::CaseInsensitive) == 0;
	mRepeatCountIfFail = aSettings.value(CScheduler::Config::RepeatCountIfFail, "0").toInt();
	mTimeThreshold     = aSettings.value(CScheduler::Config::TimeThreshold, "0").toInt();
	mRetryTimeout      = aSettings.value(CScheduler::Config::RetryTimeout, "-1").toInt();

	mLastExecute        = QDateTime::fromString(aUserSettings.value(CScheduler::UserConfig::LastExecute).toString(), CScheduler::DateTimeFormat);
	mFailExecuteCounter = aUserSettings.value(CScheduler::UserConfig::FailExecuteCounter, 0).toInt();

	QString timeStr = aSettings.value(CScheduler::Config::Time, "-1").toString();
	if (timeStr == CScheduler::Config::StartupTime)
	{
		mTime = QTime::currentTime().addSecs(60);
		mLastExecute = QDateTime::currentDateTime().addDays(-2);
		mOnlyOnce = true;
	}
	else if (timeStr == CScheduler::Config::AfterFirstRun)
	{
		mOnlyOnce = true;
		mPeriod = -1;
		if (mLastExecute.isValid() && !mLastExecute.isNull())
		{
			mLastExecute = QDateTime::currentDateTime().addSecs(-20);
			mTime = QTime::currentTime().addSecs(-30);
		}
		else
		{
			mTime = QTime::currentTime().addSecs(60 * 60);
		}
	}
	else
	{
		mTime = QTime::fromString(timeStr, CScheduler::TimeFormat);
		mOnlyOnce = aSettings.value(CScheduler::Config::OnlyOnce, false).toBool();
	}
}

//------------------------------------------------------------------------------
bool SchedulerService::Item::isOK() const
{
	return mTime.isValid() || mPeriod > 0;
}

//------------------------------------------------------------------------------
bool SchedulerService::Item::execute(SDK::PaymentProcessor::ITask * aTask, ILog * aLog)
{
	if (isOK() && aTask)
	{
		LOG(aLog, LogLevel::Normal, QString("[%1]: Execute").arg(mName));

		if (mFailExecuteCounter)
		{
			LOG(aLog, LogLevel::Normal, QString("[%1]: Restart #%2 time.").arg(mName).arg(mFailExecuteCounter));
		}

		mLastExecute = QDateTime::currentDateTime();

		aTask->execute();

		return true;
	}
	else
	{
		LOG(aLog, LogLevel::Error, QString("[%1]: Error create object '%2'.").arg(mName).arg(mType));

		return false;
	}
}

//------------------------------------------------------------------------------
QTimer * SchedulerService::Item::createTimer()
{
	QTimer * timer = new QTimer();
	timer->setSingleShot(true);

	// если последний запуск неудачный, то проверяем нужно ли перезапустить задачу
	if (mFailExecuteCounter && mFailExecuteCounter <= mRepeatCountIfFail && mRetryTimeout >= 0)
	{
		timer->setInterval(mRetryTimeout * 1000);
		return timer;
	}

	mFailExecuteCounter = 0;

	if (mTime.isValid()) // запуcк задачи в определенное время
	{
		QDate today = QDate::currentDate();
		QTime now = QTime::currentTime();

		int intervalToTomorrowStart = now.secsTo(QTime(23, 59, 59, 999)) + QTime(0, 0, 0, 1).secsTo(mTime);

		if (mLastExecute.date() < today) // cегодня не запуcкали
		{
			if (mTime <= now)
			{
				// пропуcтили время запуcка
				timer->setInterval(qMin(CScheduler::StartTimeIfExpired, intervalToTomorrowStart) * 1000);
			}
			else
			{
				// не пропуcтили, запуcкаем как положено
				timer->setInterval(now.secsTo(mTime) * 1000);
			}
		}
		else
		{
			// cегодня уже запуcкали - выcтавляем таймер на завтра
			timer->setInterval(intervalToTomorrowStart * 1000);
		}
	}
	else // запуcк задачи через интервалы времени
	{
		if (mPeriod > 0)
		{
			if (mTriggeredOnStart)
			{
				timer->setInterval(60 * 1000);

				mTriggeredOnStart = false;
			}
			else
			{
				timer->setInterval(mPeriod * 1000);
			}
		}
		else
		{
			// невалидный период запуcка
			delete timer;
			timer = nullptr;
		}
	}

	if (timer && mTimeThreshold > 0)
	{
		// добавляем рандомное время для запуска задачи
		timer->setInterval(timer->interval() + (qrand() * mTimeThreshold / RAND_MAX) * 1000);
	}

	return timer;
}

//------------------------------------------------------------------------------
void SchedulerService::Item::complete(bool aComplete)
{
	if (aComplete)
	{
		mFailExecuteCounter = 0;

		if (mOnlyOnce)
		{
			// делаем задачу невалидной, что бы больше не запускалась
			mTime = QTime();
			mPeriod = -1;
		}
	}
	else
	{
		++mFailExecuteCounter;
	}
}

//------------------------------------------------------------------------------
void SchedulerService::finishInitialize()
{
	mThread.start();
}

//------------------------------------------------------------------------------
void SchedulerService::scheduleAll()
{
	foreach (auto key, mItems.keys())
	{
		schedule(mItems[key]);
	}
}

//------------------------------------------------------------------------------
bool SchedulerService::schedule(SchedulerService::Item & aItem) const
{
	QTimer * timer = aItem.createTimer();
	if (timer)
	{
		timer->setObjectName(aItem.name());

		toLog(LogLevel::Normal, QString("[%1] scheduled to '%2'.")
			.arg(aItem.name()).arg(QDateTime::currentDateTime().addMSecs(timer->interval()).toString(CScheduler::DateTimeFormat)) );

		connect(timer, SIGNAL(timeout()), this, SLOT(execute()));
		timer->start();
		return true;
	}
	else
	{
		if (!aItem.onlyOnce())
		{
			toLog(LogLevel::Error, QString("Error of scheduling [%1].").arg(aItem.name()));
		}
	}

	return false;
}

//------------------------------------------------------------------------------
void SchedulerService::execute()
{
	QTimer * timer = dynamic_cast<QTimer*>(sender());
	if (timer)
	{
		timer->stop();

		if (mItems.contains(timer->objectName()))
		{
			SchedulerService::Item & item = mItems[timer->objectName()];
			timer->deleteLater();

			SDK::PaymentProcessor::ITask * task = mFactory[item.type()](item.name(), CScheduler::LogName, item.params());

			if (task)
			{
				{
					QWriteLocker locker(&mLock);

					mWorkingTasks.insert(item.name(), task);
				}

				task->subscribeOnComplete(this, SLOT(onTaskComplete(const QString &, bool)));

				if (!item.execute(task, getLog()))
				{
					onTaskComplete(item.name(), false);
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
void SchedulerService::onTaskComplete(const QString & aName, bool aComplete)
{
	SDK::PaymentProcessor::ITask * task = nullptr;

	{
		QWriteLocker locker(&mLock);

		task = mWorkingTasks.value(aName);
		mWorkingTasks.remove(aName);
	}

	try
	{
		QObject * taskObject = dynamic_cast<QObject *>(task);
		
		if (taskObject)
		{
			taskObject->deleteLater();
		}
		else
		{
			delete task;
		}

		task = nullptr;
	}
	catch(...) {}

	mItems[aName].complete(aComplete);

	if (aComplete)
	{
		toLog(LogLevel::Normal, QString("[%1]: Done.").arg(aName));
	}
	else 
	{
		toLog(LogLevel::Error, QString("[%1]: Error executing.").arg(aName));
	}

	saveLastExecute(mItems[aName], aComplete);

	schedule(mItems[aName]);
}

//------------------------------------------------------------------------------
void SchedulerService::saveLastExecute(Item & aItem, bool aComplete)
{
	QSettings settings(IApplication::toAbsolutePath(IApplication::getWorkingDirectory() + CScheduler::UserConfigName), QSettings::IniFormat);

	settings.beginGroup(aItem.name());
	settings.setValue(CScheduler::UserConfig::LastExecute, aItem.lastExecute().toString(CScheduler::DateTimeFormat));
	settings.setValue(CScheduler::UserConfig::FailExecuteCounter, aItem.failCount());
	settings.endGroup();
}

//---------------------------------------------------------------------------
bool SchedulerService::canShutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool SchedulerService::shutdown()
{
	{
		QReadLocker locker(&mLock);

		foreach (auto task, mWorkingTasks.values())
		{
			task->cancel();
		}
	}

	SafeStopServiceThread(&mThread, 3000, getLog());

	return true;
}

//---------------------------------------------------------------------------
QString SchedulerService::getName() const
{
	return CServices::SchedulerService;
}

//---------------------------------------------------------------------------
const QSet<QString> & SchedulerService::getRequiredServices() const
{
	//TODO Как отслеживать зависимости тасков от нужных сервисов?
	static QSet<QString> requiredServices = QSet<QString>()
		<< CServices::SettingsService
		<< CServices::EventService
		<< CServices::TerminalService
		<< CServices::RemoteService
		<< CServices::NetworkService;

	return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap SchedulerService::getParameters() const
{
	return QVariantMap();
}

//---------------------------------------------------------------------------
void SchedulerService::resetParameters(const QSet<QString> &)
{
}

//---------------------------------------------------------------------------
void SchedulerService::setupDisplayOnOff()
{
	PPSDK::TerminalSettings * terminalSettings = static_cast<PPSDK::TerminalSettings *>(mApplication->getCore()->getSettingsService()->
		getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

	QString energySave = terminalSettings->energySave();

	if (energySave.split(";", QString::SkipEmptyParts).size() >= 2)
	{
		toLog(LogLevel::Normal, QString("Enabled energy saving: %1.").arg(energySave));

		QSettings settings;
		settings.beginGroup(CScheduler::DisplayOnOffTaskName);
		settings.setValue(CScheduler::Config::Type, "OnOffDisplay");
		settings.setValue(CScheduler::Config::Period, "300");
		settings.setValue(CScheduler::Config::Params, energySave);

		QSettings userSettings(IApplication::toAbsolutePath(IApplication::getWorkingDirectory() + CScheduler::UserConfigName), QSettings::IniFormat);
		userSettings.beginGroup(CScheduler::DisplayOnOffTaskName);

		Item item(CScheduler::DisplayOnOffTaskName, settings, userSettings);
		mItems.insert(item.name(), item);
		toLog(LogLevel::Normal, QString("[%1]: Loaded.").arg(item.name()));
	}
}

//---------------------------------------------------------------------------
