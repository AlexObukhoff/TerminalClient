/* @file Реализация сторожевого сервиса как обычного приложения. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtCore/QMap>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>
#include <MessageQueue/IMessageQueueServer.h>
#include <SysUtils/ISysUtils.h>

// Project
#include "SplashScreen.h"
#include "TimeChangeListener.h"

//----------------------------------------------------------------------------
namespace CStartMode
{
	const char Normal[] = "normal";
	const char Exclusive[] = "exclusive";
}

//----------------------------------------------------------------------------
namespace CWatchService
{
	const int CheckInterval = 3 * 1000; // 3 sec
	const int ScreenActivityTimeout = 5 * 1000; // 5 sec

	const int KillModuleTimeout = 30; // in sec
	const int CheckMemoryUsageTimeout = 60; // in min

	namespace SlowPC
	{
		const int Threshold = 1400; // in MHz

		const int KillModuleTimeout = 180; // in sec
	}

	const uint FirstPingTimeoutDefault = 60; // in sec
	const uint FirstPingTimeoutIncrement = 10; // in sec
	const uint FirstPingTimeoutMax = 600; // in sec
}

//----------------------------------------------------------------------------
/// Действие, которое будет выполнено после закрытия сервиса.
namespace ECloseAction
{
	enum Enum
	{
		None = 0, /// Никаких специальных действий.
		Restart,  /// Перезапуск модулей.
		Exit,     /// Закрытие сервиса.
		Reboot,   /// Перезагрузка системы.
		Shutdown  /// Выключение системы.
	};
}

//----------------------------------------------------------------------------
struct SModule
{
	QString name;
	QString file;
	QString workingDirectory;
	QString params;
	QString startMode;
	QDateTime initDate;
	QDateTime lastUpdate;
	uint restartCount;
	QProcess * process;
	bool autoStart;
	uint startPriority;
	uint closePriority;
	uint afterStartDelay;
	bool needToStart;
	bool previousNeedToStart;
	bool gui;
	uint firstPingTimeout;
	QByteArray lastAnswer;
	uint maxStartCount;
	QStringList commandStack;
	QString arguments;
	int noResponseCount;
	ISysUtils::MemoryInfo memoryUsage;
	int killTimeout;
	int killOnStartCount;

	SModule();

	/// Модуль прислал сигнал alive
	void touch();

	/// Прибить модуль 
	bool kill();

	/// Получить таймаут до первого пинга модуля
	int getFirstPingTimeout() const;
};

//----------------------------------------------------------------------------
class WatchService : public QObject, protected ILogable
{
	Q_OBJECT

public:
	WatchService();

	virtual ~WatchService();

	/// Возвращает таймаут, в зависимости от скорости процессора
	static int defaultKillTimeout();

signals:
	/// Сигнал срабатывает, когда не запущен ни один модуль с графическим интерфейсом.
	void screenUnprotected();

	/// Сигнал срабатывает, когда запущен модуль с графическим интерфейсом.
	void screenProtected();

	/// Сигнал об изменении состояния модуля aSender.
	void stateChanged(QString aSender, QString aState);

	/// Сигнал о сбросе состояния модуля aSender.
	void stateReset(QString aSender);

protected:
	/// Чтение конфигурации о модулях.
	virtual void loadConfiguration();

	/// От одного из модулей пришло сообщение.
	virtual void messageReceived(const QByteArray & aMessage);

	/// Запустить процедуру перезагрузки терминала
	void doReboot();

	/// Работа всех модулей была завершена.
	virtual void modulesClosed();

	/// Событие при завершении всех модулей.
	virtual void closeAction();

public slots:
	/// Инициализация сервиса.
	virtual void initialize();

	/// Останавливаем сервисы по запросу операционной системы
	void closeBySystemRequest(QSessionManager & aSessionManager);

private slots:
	/// Слот закрытие всех работающих модулей.
	void closeModules();

	/// Проверка состояния модулей.
	void onCheckModules();

	/// Обработка клика по защитному экрану.
	void onScreenActivity(int aArea);

	/// Обнуляем накопленные экраные клики
	void onScreenActivityTimeout();

	/// От одного из модулей пришло сообщение.
	void onMessageReceived(QByteArray aMessage);

	/// Один из модулей был закрыт.
	void onModuleFinished(int aExitCode, QProcess::ExitStatus aExitStatus);

	/// Обработчик сигнала об изменении системного времени 
	void onTimeChanged(qint64 aTimeOffset);

	/// Проверка используемой памяти процессом
	void checkProcessMemory();

	/// Запсутить все autoStart модули
	void checkAutoStartModules();

private:
	/// Запустить модуль
	void startModule(SModule & aModule);

	/// Закрытие всех работающих модулей.
	void doCloseModules(QString aSender);

	/// Проверка защищённости рабочего стола.
	void checkScreenProtection();

	/// Отключение проверки рабочего стола.
	void enableScreenProtection(bool aEnabled);

	/// Закрытие модуля по приоритету или экстренно.
	bool closeModule(SModule & aModule, bool aIgnorePriority = false);

	bool canRun(const SModule & aModule);

	bool canTerminate(const SModule & aModule);

	/// Перевод кода ошибки процесса в строку.
	QString translateError(QProcess::ProcessError aError);

	/// Пересылка команды неизвестному модулю
	void sendCommandToUknownModule(const QString & aCommand, const QString & aModule);

private slots:
	/// Реинициализация сервиса
	void reinitialize();

private:
	typedef QMap<QString, SModule> TModules;

	QSharedPointer<IMessageQueueServer> mServer;

	/// При первом запуске модулей всегда true.
	bool mFirstRun;

	/// Защитный экран.
	SplashScreen mSplashScreen;

	/// Таймер на проверку состояния модулей.
	QTimer mTimer;

	/// Таймер для обнуления экранных кликов
	QTimer mScreenActivityTimer;

	/// Последовательность экранных кликов
	QString mClickSequence;

	/// Это действие будет выполнено после закрытия всех модулей.
	ECloseAction::Enum mCloseAction;

	/// Включён ли автоматический показ сплэш-скрина
	bool mScreenProtectionEnabled;

	TModules mModules;

#if 0 // #40592 Пока выключаем данную опцию
private slots:
	/// Остановка всех запрещённых процессов
	void checkForbiddenModules();

	/// Запустить процессы, остановленные при старте ТК
	void startTerminatedModules();

private:
	/// Флаг - признак выключения модулей при старте ТК
	bool mShutdownForbiddenModules;

	/// Список процессов подлежащих остановке во время работы ТК
	QStringList mForbiddenModules;

	/// Список процессов остановленных при старте
	QStringList mTerminatedModules;
	int mCheckForbiddenTimeout;
	QSharedPointer<QTimer> mCheckForbiddenTimer;
#endif

private:
	/// Параметры при перезапуске 
	QString mRestartParameters;

	QSharedPointer<TimeChangeListener> mTimeChangeListner;
	QSharedPointer<QTimer> mCheckMemoryTimer;

	int mInitializeFailedCounter;
};

//----------------------------------------------------------------------------
