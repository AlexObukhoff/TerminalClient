/* @file Сценарий простоя. */

#pragma once

// SDK
#include <SDK/PaymentProcessor/Core/Event.h>

// modules
#include <ScenarioEngine/Scenario.h>

class IApplication;

//---------------------------------------------------------------------------
namespace CGUISignals
{
	const char StartGUI[] = "start_ui";
	const char StopGUI[] = "stop_ui";
	const char UpdateGUI[] = "update_ui";
	const char ScreenPasswordUpdated[] = "screen_password_updated";
}

//---------------------------------------------------------------------------
class IdleScenario : public GUI::Scenario
{
	Q_OBJECT
	Q_ENUMS(Command)

public:
	enum Command
	{
		None,
		Autoencashment
	};

public:
	IdleScenario(IApplication * mApplication);
	virtual ~IdleScenario();

	/// Запуск сценария.
	virtual void start(const QVariantMap & aContext);

	/// Остановка сценария.
	virtual void stop() {};

	/// Приостановить сценарий с возможностью последующего возобновления.
	virtual void pause();

	/// Продолжение после паузы.
	virtual void resume(const QVariantMap & aContext);

	/// Инициализация сценария.
	virtual bool initialize(const QList<GUI::SScriptObject> & aScriptObjects);

	/// Обработка сигнала из активного состояния с дополнительными аргументами.
	virtual void signalTriggered(const QString & aSignal, const QVariantMap & aArguments = QVariantMap());

	/// Возвращает false, если сценарий не может быть остановлен в текущий момент.
	virtual bool canStop();

public slots:
	/// Текущее состояние.
	virtual QString getState() const;

	/// Обработчик таймаута
	virtual void onTimeout();

private:
	/// Определяем новое состояние.
	void updateState(const QString & aSignal, const QVariantMap & aParameters);

	/// Выполнение команды.
	void execCommand();

private slots:
	/// Обработчик события.
	void onEvent(const SDK::PaymentProcessor::Event & aEvent);

private:
	IApplication * mApplication;
	QString mDefaultScenario;
	Command mCommand;
	bool mActive;
	bool mNoGui;
};

//---------------------------------------------------------------------------
