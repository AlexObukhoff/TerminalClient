/* @file Плагин сценария для автоматической миграции с 2.х.х на 3.х.х версию терминального ПО */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QSharedPointer>
#include <QtCore/QFutureWatcher>
#include <QtCore/QReadWriteLock>
#include <Common/QtHeadersEnd.h>

#include <SDK/PaymentProcessor/Connection/Connection.h>

// Modules
#include <ScenarioEngine/Scenario.h>

// Plugin SDK
#include <SDK/Plugins/IFactory.h>

// Project

class IApplication;

namespace SDK {
	namespace PaymentProcessor {
		class ICore;
		class INetworkService;
		class ITerminalService;
		class ISettingsService;
		class ICryptService;
		class TerminalSettings;
		class IDeviceService;

		namespace Scripting {
			class Core;
		}
	}

	namespace Plugin {
		class IEnvironment;
	}
}

namespace Migrator3000
{

//---------------------------------------------------------------------------
struct STerminal
{
	struct SConnection
	{
		QString type;
		QString name;

		// proxy
		QString username;
		QString password;
		QString host;
		QString proxy_type;
		size_t port;

		SConnection() : port(0) {}

		bool isEmpty() { return name.isEmpty() && type.isEmpty(); }
	};

	struct SPort
	{
		size_t baud_rate;
		QString number;
		size_t parity;
		QString type;
		QString uuid;

		SPort() : baud_rate(0), parity(0) {}
	};

	struct SDevice
	{
		QString type;
		QString model;
		QString vendor;

		size_t fiscal_mode;

		QString uuid;

		SPort port;

		SDevice() : fiscal_mode(0) {}
	};

	bool isEmpty() { return connection.isEmpty(); }

	typedef std::vector<SDevice> TDevices;
	TDevices devices;
	SConnection connection;
	SDK::PaymentProcessor::SConnection connection2;
};

//---------------------------------------------------------------------------
class MainScenario : public GUI::Scenario
{
	Q_OBJECT

public:
	MainScenario(SDK::PaymentProcessor::ICore * aCore, ILog * aLog);
	virtual ~MainScenario();

public:
	/// Запуск сценария.
	virtual void start(const QVariantMap & aContext);

	/// Остановка сценария.
	virtual void stop();

	/// Приостановить сценарий с возможностью последующего возобновления.
	virtual void pause();

	/// Продолжение после паузы.
	virtual void resume(const QVariantMap & aContext);

	/// Инициализация сценария.
	virtual bool initialize(const QList<GUI::SScriptObject> & aScriptObjects);

	/// Обработка сигнала из активного состояния с дополнительными аргументами.
	virtual void signalTriggered(const QString & aSignal, const QVariantMap & aArguments = QVariantMap());

	/// Обработчик таймаута
	virtual void onTimeout();

	/// Возвращает false, если сценарий не может быть остановлен в текущий момент.
	virtual bool canStop();

	public slots:
	/// Текущее состояние.
	virtual QString getState() const;

	private slots:
	void onTaskFinished();

private slots:
	/// Посылается, когда обнаружено очередное устройство.
	void onDeviceDetected(const QString & aConfigName);

	/// Посылается когда процесс обнаружения устройств останавливается.
	void onDetectionStopped();

	void finishDeviceDetection();

private:

private:
	QVariantMap mContext;
	QString mLastSignal;
	SDK::PaymentProcessor::ICore * mCore;
	SDK::PaymentProcessor::ISettingsService * mSettingsService;
	SDK::PaymentProcessor::INetworkService * mNetworkService;
	SDK::PaymentProcessor::ITerminalService * mTerminalService;
	SDK::PaymentProcessor::TerminalSettings * mTerminalSettings;
	SDK::PaymentProcessor::ICryptService * mCryptService;
	SDK::PaymentProcessor::Scripting::Core * mScriptingCore;
	SDK::PaymentProcessor::IDeviceService * mDeviceService;

	QFutureWatcher<bool> mTaskWatcher;

	QStringList mFoundedDevices;
	int mMonitoringComandId;

	//
	QString mKiosk2InstallPath;
};

}

//--------------------------------------------------------------------------

