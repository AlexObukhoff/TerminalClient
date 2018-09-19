/* @file Реализация менеджера сетевых соединений. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

// Модули
#include <Common/ExceptionFilter.h>

// SDK
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Settings/Directory.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/Watchdog/LineTypes.h>

// Проект
#include "System/IApplication.h"

#include "DatabaseUtils/IHardwareDatabaseUtils.h"

#include "Services/ServiceNames.h"
#include "Services/SettingsService.h"
#include "Services/DatabaseService.h"
#include "Services/DeviceService.h"
#include "Services/ServiceCommon.h"

#include "NetworkService.h"

namespace PP = SDK::PaymentProcessor;

Q_DECLARE_METATYPE(SDK::PaymentProcessor::SConnection);
Q_DECLARE_METATYPE(bool *);

//---------------------------------------------------------------------------
namespace CNetworkService
{
	// Ожидание после неудачного подключения.
	const int RestoreTimeout = 3 * 60 * 1000;

	// Кол-во попыток перед перезагрузкой. (t = FailsBeforeReboot * RestoreTimeout мс.)
	const int FailsBeforeReboot = 10;

	// Время повторной проверки состояния соединения в случае dialup
	const int CheckConnectionInterval = 3 * 60 * 1000;

	// Интервал между обновлениями параметров модема.
	const int UpdateStatusInterval = 12 * 60 * 60 * 1000;

	// Интервал ожидания переустановки соединения, после сброса модема сторожом, с.
	const int ReestablishInterval = 30;

	// Время, в течении которого наблюдаются постоянные сетевые ошибки, приводящее к решению об обрыве связи (в минутах)
	const int NetworkFailureTimeout = 3;
}

//---------------------------------------------------------------------------
NetworkService::NetworkService(IApplication * aApplication)
	: ILogable("Connection"),
	  mDeviceService(0),
	  mEventService(0),
	  mConnection(0),
	  mEnabled(true),
	  mDontWatchConnection(false),
	  mApplication(aApplication),
	  mFails(0)
{
	QObject::moveToThread(this);

	setObjectName(CServices::NetworkService);

	mRestoreTimer.setSingleShot(true);
	mRestoreTimer.moveToThread(this);
	connect(&mRestoreTimer, SIGNAL(timeout()), SLOT(onConnectionLost()));

	mParametersUpdateTimer.moveToThread(this);
	mParametersUpdateTimer.setInterval(CNetworkService::UpdateStatusInterval);
	connect(&mParametersUpdateTimer, SIGNAL(timeout()), SLOT(updateModemParameters()));

	mNetworkTaskManager.setLog(getLog());
	connect(&mNetworkTaskManager, SIGNAL(networkTaskStatus(bool)), this, SLOT(onNetworkTaskStatus(bool)), Qt::QueuedConnection);

	qRegisterMetaType<SDK::PaymentProcessor::SConnection>();
	qRegisterMetaType<bool *>();
}

//---------------------------------------------------------------------------
NetworkService::~NetworkService()
{
}

//---------------------------------------------------------------------------
bool NetworkService::initialize()
{
	mDeviceService = mApplication->getCore()->getDeviceService();

	mEventService = mApplication->getCore()->getEventService();

	PP::TerminalSettings * terminalSettings = SettingsService::instance(mApplication)->getAdapter<PP::TerminalSettings>();

	mDatabaseUtils = DatabaseService::instance(mApplication)->getDatabaseUtils<IHardwareDatabaseUtils>();

	// Активируем сохраненное в конфиге соединение.
	mConnectionSettings = terminalSettings->getConnection();

	// Создаем и запускаем поток сетевого сервиса.
	start();

	return true;
}

//------------------------------------------------------------------------------
void NetworkService::finishInitialize()
{
}

//---------------------------------------------------------------------------
bool NetworkService::canShutdown()
{
	// Если нас спросили, можем ли выгрузится - это жжж не спроста. 
	// Поэтому закрываем все текущие сетевые задачи.
	mNetworkTaskManager.clearTasks();

	return true;
}

//---------------------------------------------------------------------------
bool NetworkService::shutdown()
{
	mEnabled = false;
	mDontWatchConnection = true;

	// Останавливаем сетевой поток и дожидаемся его остановки.
	SafeStopServiceThread(&mNetworkTaskManager, 10000, getLog());

	// Останавливаем сервисный поток и дожидаемся его остановки.
	SafeStopServiceThread(this, 3000, getLog());

	return true;
}

//---------------------------------------------------------------------------
QString NetworkService::getName() const
{
	return CServices::NetworkService;
}

//---------------------------------------------------------------------------
QVariantMap NetworkService::getParameters() const
{
	QVariantMap parameters;

	if (mSignalLevel.is_initialized())
	{
		parameters[PP::CServiceParameters::Networking::SignalLevel] = mSignalLevel.get();
	}

	if (mBalance.is_initialized())
	{
		parameters[PP::CServiceParameters::Networking::SimBalance] = mBalance.get();
	}

	if (mOperator.is_initialized())
	{
		parameters[PP::CServiceParameters::Networking::Provider] = mOperator.get();
	}

	return parameters;
}

//---------------------------------------------------------------------------
void NetworkService::resetParameters(const QSet<QString> &)
{
}

//---------------------------------------------------------------------------
const QSet<QString> & NetworkService::getRequiredServices() const
{
	static QSet<QString> requiredServices = QSet<QString>()
		<< CServices::DeviceService
		<< CServices::EventService
		<< CServices::DatabaseService
		<< CServices::SettingsService;

	return requiredServices;
}

//---------------------------------------------------------------------------
bool NetworkService::getConnectionTemplate(const QString & aConnectionName, PP::SConnectionTemplate & aConnectionTemplate) const
{
	PP::Directory * directory = SettingsService::instance(mApplication)->getAdapter<PP::Directory>();

	foreach (PP::SConnectionTemplate connectionTemplate, directory->getConnectionTemplates())
	{
		if (connectionTemplate.name == aConnectionName)
		{
			aConnectionTemplate = connectionTemplate;
			return true;
		}
	}

	return false;
}

//---------------------------------------------------------------------------
NetworkTaskManager * NetworkService::getNetworkTaskManager()
{
	return &mNetworkTaskManager;
}

//---------------------------------------------------------------------------
bool NetworkService::openConnection(bool aWait)
{
	QMetaObject::invokeMethod(this, "doConnect", aWait ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
		Q_ARG(const SDK::PaymentProcessor::SConnection &, mConnectionSettings));

	return true;
}

//---------------------------------------------------------------------------
bool NetworkService::closeConnection()
{
	return QMetaObject::invokeMethod(this, "doDisconnect", Qt::BlockingQueuedConnection);
}

//---------------------------------------------------------------------------
bool NetworkService::isConnected(bool aUseCache)
{
	try
	{
		return mConnection ? mConnection->isConnected(aUseCache) : false;
	}
	catch (const NetworkError & e)
	{
		toLog(LogLevel::Error, e.getMessage());
		if (e.getSeverity() == ESeverity::Critical)
		{
			toLog(LogLevel::Fatal, "Generating reboot event due to critical error.");
			mEventService->sendEvent(PP::Event(PP::EEventType::Reboot));
		}

		QMutexLocker lock(&mErrorMutex);
		mLastConnectionError = e.getMessage();

		return false;
	}
}

//---------------------------------------------------------------------------
void NetworkService::setConnection(const PP::SConnection & aConnection)
{
	toLog(LogLevel::Normal, QString("Setting new connection '%1'.").arg(aConnection.name));

	// Страхуемся от изменений mConnection
	PP::SConnection connectionSettings = getConnection();

	if (connectionSettings == aConnection)
	{
		toLog(LogLevel::Normal, "Already set up with an indentical connection.");
		return;
	}

	// Запоминаем, установлено или нет соединение.
	if (isConnected())
	{
		// Разрываем старое соединение.
		QMetaObject::invokeMethod(this, "doDisconnect", Qt::QueuedConnection);
	}

	// Устанавливаем новое соединение.
	QMetaObject::invokeMethod(this, "doConnect", Qt::QueuedConnection, Q_ARG(const SDK::PaymentProcessor::SConnection &, aConnection));
}

//---------------------------------------------------------------------------
PP::SConnection NetworkService::getConnection() const
{
	return mConnectionSettings;
}

//---------------------------------------------------------------------------
bool NetworkService::testConnection()
{
	bool result = false;
	QMetaObject::invokeMethod(this, "doTestConnection", Qt::BlockingQueuedConnection, Q_ARG(bool *, &result));
	return result;
}

//---------------------------------------------------------------------------
void NetworkService::doTestConnection(bool * aResult)
{
	// Ожидаем установки соединения.
	if (!isConnected())
	{
		doConnect(getConnection());
	}

	toLog(LogLevel::Normal, QString("Testing connection '%1'...").arg(mConnection->getName()));

	try
	{
		*aResult = mConnection->isConnected() && mConnection->checkConnection();
	}
	catch (const NetworkError & e)
	{
		toLog(LogLevel::Error, e.getMessage());

		if (e.getSeverity() == ESeverity::Critical)
		{
			toLog(LogLevel::Fatal, "Generating reboot event due to critical error.");
			mEventService->sendEvent(PP::Event(PP::EEventType::Reboot));
		}

		*aResult = false;

		QMutexLocker lock(&mErrorMutex);
		mLastConnectionError = e.getMessage();
	}
}

//---------------------------------------------------------------------------
void NetworkService::updateModemParameters()
{
	if (getConnection().type != EConnectionTypes::Dialup || !mEnabled)
	{
		return;
	}

	toLog(LogLevel::Normal, "Updating GPRS connection parameters...");

	// Разрываем соединение для доступа к модему.
	doDisconnect();

	// Получаем параметры.
	SDK::Driver::IModem * modemDevice = prepareModem(getModem(), "");

	if (modemDevice)
	{
		mBalance.reset();
		mOperator.reset();
		mSignalLevel.reset();

		PP::SConnectionTemplate connectionTemplate;

		if (getConnectionTemplate(getConnection().name, connectionTemplate))
		{
			QString reply;

			// Выполняем USSD-запрос и извлекаем из него строку с информацией о балансе.
			if (modemDevice->processUSSD(connectionTemplate.balanceNumber, reply))
			{
				QRegExp regExp(connectionTemplate.regExp);

				if (regExp.indexIn(reply) != -1)
				{
					mBalance = regExp.cap(0);
					mDatabaseUtils->setDeviceParam(mDeviceService->getDeviceConfigName(modemDevice), PP::CDatabaseConstants::Parameters::BalanceLevel, mBalance.get());
				}
				else
				{
					toLog(LogLevel::Error, QString("Failed to parse USSD reply: '%1'.").arg(reply));
				}
			}
			else
			{
				toLog(LogLevel::Error, QString("Failed to send USSD: '%1'.").arg(connectionTemplate.balanceNumber));
			}
		}
		else
		{
			toLog(LogLevel::Error, QString("Connection template '%1' not found. USSD request for balance undefined.").arg(getConnection().name));
		}

		QString operatorName;

		if (modemDevice->getOperator(operatorName))
		{
			mOperator = operatorName;
		}
		else
		{
			toLog(LogLevel::Error, "Failed to retrieve GSM operator info.");
			// В качестве имени оператора связи ставим имя соединения
			mOperator = getConnection().name;
		}

		mDatabaseUtils->setDeviceParam(mDeviceService->getDeviceConfigName(modemDevice), PP::CDatabaseConstants::Parameters::ConnectionName, mOperator.get());

		int signalLevel = 0;

		if (modemDevice->getSignalQuality(signalLevel))
		{
			mSignalLevel = signalLevel;
			mDatabaseUtils->setDeviceParam(mDeviceService->getDeviceConfigName(modemDevice), PP::CDatabaseConstants::Parameters::SignalLevel, mSignalLevel.get());
		}
		else
		{
			toLog(LogLevel::Error, "Failed to retrieve signal level.");
		}

		mDatabaseUtils->setDeviceParam(mDeviceService->getDeviceConfigName(modemDevice), PP::CDatabaseConstants::Parameters::LastCheckBalanceTime, QDateTime::currentDateTime());

		if (mBalance.is_initialized())
		{
			toLog(LogLevel::Normal, QString("Balance is: %1").arg(mBalance.get()));
		}
		else
		{
			toLog(LogLevel::Error, "Get balance error.");
		}

		QString modemInfo;

		if (modemDevice->getInfo(modemInfo))
		{
			toLog(LogLevel::Normal, QString("Modem info: %1").arg(modemInfo));
			mDatabaseUtils->setDeviceParam(mDeviceService->getDeviceConfigName(modemDevice), PP::CDatabaseConstants::Parameters::DeviceInfo, modemInfo);
		}

		SDK::Driver::IModem::TMessages messages;

		if (modemDevice->takeMessages(messages))
		{
			foreach (auto sms, messages)
			{
				toLog(LogLevel::Normal, QString("SMS [%1] at %2: %3.").arg(sms.from).arg(sms.date.toString()).arg(sms.text));
			}
		}
	}
	else
	{
		toLog(LogLevel::Error, "Failed to retrieve modem parameters.");
	}

	// Восстанавливаем соединение.
	QMetaObject::invokeMethod(this, "doConnect", Qt::QueuedConnection, Q_ARG(const SDK::PaymentProcessor::SConnection &, mConnectionSettings));
}

//---------------------------------------------------------------------------
void NetworkService::doConnect(const SDK::PaymentProcessor::SConnection & aConnection)
{
	if (!mEnabled)
	{
		return;
	}

	try
	{
		toLog(LogLevel::Normal, QString("Attempt %2: establishing connection '%1'...").arg(aConnection.name).arg(mFails));

		mConnection = QSharedPointer<IConnection>(IConnection::create(aConnection.name, aConnection.type, &mNetworkTaskManager, 
			mApplication->getLog()->getInstance("Connection")));

		mConnection->setCheckPeriod(aConnection.checkInterval);

		// Всегда читаем список серверов для проверки соединения из конфигов.
		auto settings = SettingsService::instance(mApplication)->getAdapter<PP::TerminalSettings>();
		mConnection->setCheckHosts(settings->getCheckHosts());
		connect(mConnection.data(), SIGNAL(connectionLost()), SLOT(onConnectionLost()), Qt::QueuedConnection);
		connect(mConnection.data(), SIGNAL(connectionAlive()), SLOT(onConnectionAlive()), Qt::QueuedConnection);

		mConnectionSettings = aConnection;

		// Попытка поднять соединение.
		if (!isConnected())
		{
			if (aConnection.type == EConnectionTypes::Dialup)
			{
				if (!prepareModem(getModem(), aConnection.name))
				{
					toLog(LogLevel::Warning, "Failed to initialize modem device.");
				}
			}
		}
		
		// Открываем соединение в любом случае
		mConnection->open();

		if (isConnected())
		{
			// Устанавливаем прокси-сервер.
			if (getConnection().type == EConnectionTypes::Dialup)
			{
				toLog(LogLevel::Normal, "Proxy disabled.");
				mNetworkTaskManager.setProxy(QNetworkProxy::NoProxy);
			}
			else
			{
				toLog(LogLevel::Normal, QString("Using proxy: host = %1, port = %2.").arg(getConnection().proxy.hostName()).arg(getConnection().proxy.port()));
				mNetworkTaskManager.setProxy(getConnection().proxy);
			}

			// Посылаем сообщение о том, что связь восстановлена.
			mEventService->sendEvent(PP::Event(PP::EEventType::ConnectionEstablished));

			toLog(LogLevel::Normal, QString("Connected to '%1'.").arg(aConnection.name));
		}
		else
		{
			toLog(LogLevel::Error, QString("Failed to connect to '%1'.").arg(aConnection.name));
		}

		//TODO #29565 - проверяем статус модема, и в случае ошибок выставляем ему статус OK - Connection enstablished
		if (isConnected() && getConnection().type == EConnectionTypes::Dialup)
		{
			auto modem = getModem();
			if (modem)
			{
				DeviceService::instance(mApplication)->overwriteDeviceStatus(modem, SDK::Driver::EWarningLevel::OK, "Connected", 0);
			}
		}
	}
	catch (const NetworkError & e)
	{
		toLog(LogLevel::Error, e.getMessage());

		if (e.getSeverity() == ESeverity::Critical)
		{
			toLog(LogLevel::Fatal, "ConnectionManager: generating reboot event due to critical error.");

			mEventService->sendEvent(PP::Event(PP::EEventType::Reboot));
		}

		QMutexLocker lock(&mErrorMutex);
		mLastConnectionError = e.getMessage();
	}

	if (!isConnected())
	{
		// Не получилось, увеличиваем счётчики попыток
		++mFails;

		// Ожидание перед следующим подключением линейно увеличивается
		mRestoreTimer.start(CNetworkService::RestoreTimeout);
	}
	else
	{
		mRestoreTimer.stop();
	}
}

//---------------------------------------------------------------------------
void NetworkService::checkConnection()
{
	try
	{
		if (!isConnected())
		{
			toLog(LogLevel::Warning, "Forced connection attempt.");

			QMetaObject::invokeMethod(this, "doConnect", Qt::QueuedConnection, Q_ARG(const SDK::PaymentProcessor::SConnection &, mConnectionSettings));
		}
	}
	catch (const NetworkError & e)
	{
		toLog(LogLevel::Error, e.getMessage());

		if (e.getSeverity() == ESeverity::Critical)
		{
			toLog(LogLevel::Fatal, "Generating reboot event due to critical error.");
			mEventService->sendEvent(PP::Event(PP::EEventType::Reboot));
		}

		QMutexLocker lock(&mErrorMutex);
		mLastConnectionError = e.getMessage();
	}
}

//---------------------------------------------------------------------------
void NetworkService::run()
{
	try
	{
		toLog(LogLevel::Normal, "NetworkService: Service thread started.");

		// Запускаем таймер на проверку параметров модема.
		mParametersUpdateTimer.start();

		SDK::Driver::IModem * modem = getModem();

		if (getConnection().type == EConnectionTypes::Dialup && modem)
		{
			modem->subscribe(SDK::Driver::IDevice::InitializedSignal, this, SLOT(onModemInitialized()));

			// если модем не инициализируется за 3 минуты запустим соединение принудительно
			QTimer::singleShot(CNetworkService::CheckConnectionInterval, this, SLOT(checkConnection()));
		}
		else
		{
			doConnect(getConnection());
		}

		QThread::exec();

		if (isConnected())
		{
			doDisconnect();
		}
	}
	catch (...)
	{
		EXCEPTION_FILTER_NO_THROW(getLog());
	}

	mEventService->sendEvent(PP::Event(PP::EEventType::ConnectionLost));
}

//---------------------------------------------------------------------------
void NetworkService::onModemInitialized()
{
	// Запускаем первую проверку параметров модема
	QMetaObject::invokeMethod(this, "updateModemParameters", Qt::QueuedConnection);
}

//---------------------------------------------------------------------------
void NetworkService::onConnectionAlive()
{
	// В случае успешной проверки связи - сбрасываем счетчик обрывов
	mFails = 0;
}

//---------------------------------------------------------------------------
void NetworkService::onConnectionLost()
{
	toLog(LogLevel::Warning, "Connection lost.");

	if (mDontWatchConnection || !mEnabled)
	{
		return;
	}

	// Увеличиваем счётчик обрыва связи
	++mFails;

	// Посылаем сообщение о том, что связи сейчас нет
	mEventService->sendEvent(PP::Event(PP::EEventType::ConnectionLost));

	doDisconnect();

	// Возможно, нужно перегузить терминал.
	if (mFails >= CNetworkService::FailsBeforeReboot)
	{
		toLog(LogLevel::Warning, QString("Generating system reboot event after %1 unsuccessful tries to establish connection.").arg(mFails));

		// WARNING: если WatchService не отработал (standalone-режим), соединение не будет восстанавливаться.
		mEventService->sendEvent(PP::Event(PP::EEventType::Reboot));
		return;
	}

	//TODO #29565 - проверяем статус модема, и в случае OK выставляем ему статус Warning - Disconnected
	if (getConnection().type == EConnectionTypes::Dialup)
	{
		auto modem = getModem();
		if (modem)
		{
			DeviceService::instance(mApplication)->overwriteDeviceStatus(modem, SDK::Driver::EWarningLevel::Warning, "Disconnected", 0);
		}
	}

	// Или сбросить модем сторожевиком.
	if (getConnection().type == EConnectionTypes::Dialup)
	{
		// Сбрасываем модем посредством сторожа.
		PP::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PP::TerminalSettings>();

		QStringList watchdogConfigName = settings->getDeviceList().filter(SDK::Driver::CComponents::Watchdog);

		SDK::Driver::IWatchdog * watchdogDevice = 0;

		if (!watchdogConfigName.empty())
		{
			watchdogDevice = dynamic_cast<SDK::Driver::IWatchdog *>(mDeviceService->acquireDevice(watchdogConfigName.first()));
		}

		if (watchdogDevice)
		{
			toLog(LogLevel::Warning, "Resetting modem with watchcdog.");

			// Сбросываем питание модема.
			if (watchdogDevice->reset(SDK::Driver::LineTypes::Modem))
			{
				// Ждем, пока модем оживет после сброса питания.
				sleep(CNetworkService::ReestablishInterval);
			}
			else
			{
				toLog(LogLevel::Error, "Failed to reset modem by watchcdog.");
			}
		}
		else
		{
			toLog(LogLevel::Error, "Failed to acquire watchdog device. Modem reset failed.");
		}
	}

	QMetaObject::invokeMethod(this, "doConnect", Qt::QueuedConnection, Q_ARG(const SDK::PaymentProcessor::SConnection &, mConnectionSettings));
}

//---------------------------------------------------------------------------
SDK::Driver::IModem * NetworkService::getModem()
{
	// Запрашиваем модем.
	PP::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PP::TerminalSettings>();

	QStringList modemConfigName = settings->getDeviceList().filter(SDK::Driver::CComponents::Modem);
	SDK::Driver::IModem * modemDevice = !modemConfigName.empty() ? dynamic_cast<SDK::Driver::IModem *>(mDeviceService->acquireDevice(modemConfigName.first())) : nullptr;

	if (!modemDevice)
	{
		toLog(LogLevel::Error, "Modem is not present.");
	}

	return modemDevice;
}

//---------------------------------------------------------------------------
SDK::Driver::IModem * NetworkService::prepareModem(SDK::Driver::IModem * aModemDevice, const QString & aConnectionName)
{
	if (aModemDevice)
	{
		// Сбрасываем модем.
		if (!aModemDevice->reset())
		{
			toLog(LogLevel::Error, "Failed to reset modem.");
		}

		PP::SConnectionTemplate connectionTemplate;

		// Инициализируем модем.
		if (getConnectionTemplate(aConnectionName, connectionTemplate))
		{
			if (!aModemDevice->setInitString(connectionTemplate.initString))
			{
				toLog(LogLevel::Error, QString("Failed to set modem initialization string: %1.").arg(connectionTemplate.initString));
			}
		}
	}

	return aModemDevice;
}

//---------------------------------------------------------------------------
bool NetworkService::doDisconnect()
{
	try
	{
		if (isConnected())
		{
			mConnection->close();
		}

		toLog(LogLevel::Normal, QString("Disconnected from '%1'.").arg(getConnection().name));

		return true;
	}
	catch (const NetworkError & e)
	{
		toLog(LogLevel::Error, e.getMessage());

		if (e.getSeverity() == ESeverity::Critical)
		{
			toLog(LogLevel::Fatal, "Generating reboot event due to critical error.");
			mEventService->sendEvent(PP::Event(PP::EEventType::Reboot));
		}

		QMutexLocker lock(&mErrorMutex);
		mLastConnectionError = e.getMessage();
	}

	return false;
}

//---------------------------------------------------------------------------
QString NetworkService::getLastConnectionError() const
{
	QMutexLocker lock(&mErrorMutex);
	return mLastConnectionError;
}

//---------------------------------------------------------------------------
void NetworkService::setUserAgent(const QString aUserAgent)
{
	mNetworkTaskManager.setUserAgent(aUserAgent);
}

//---------------------------------------------------------------------------
QString NetworkService::getUserAgent() const
{
	return mNetworkTaskManager.getUserAgent();
}

//---------------------------------------------------------------------------
void NetworkService::onNetworkTaskStatus(bool aFailure)
{
	if (!aFailure)
	{
		// при успешном обращении сбрасываем метку первой ошибки
		mNetworkTaskFailureStamp = QDateTime();
	}
	else if (mNetworkTaskFailureStamp.isNull())
	{
		// первая ошибка
		mNetworkTaskFailureStamp = QDateTime::currentDateTime();
	}
	else if (qAbs(mNetworkTaskFailureStamp.secsTo(QDateTime::currentDateTime()) / 60.) > CNetworkService::NetworkFailureTimeout)
	{
		toLog(LogLevel::Error, QString("Errors network calls for %1 minutes. Connection lost.").arg(CNetworkService::NetworkFailureTimeout));

		mNetworkTaskFailureStamp = QDateTime();

		QMetaObject::invokeMethod(this, "onConnectionLost", Qt::QueuedConnection);
	}
}

//---------------------------------------------------------------------------