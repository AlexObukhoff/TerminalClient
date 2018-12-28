/* @file Сценарий управления терминалом. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtCore/QSet>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/DeviceTypes.h>
#include <SDK/Drivers/WarningLevel.h>
#include <SDK/Drivers/CashAcceptor/CashAcceptorStatus.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Modules
#include <Common/Application.h>
#include <Common/ExceptionFilter.h>
#include <SysUtils/ISysUtils.h>
#include <WatchServiceClient/Constants.h>
#include <Packer/Packer.h>
#include <Common/Version.h>

// Project
#include "System/IApplication.h"
#include "DatabaseUtils/IHardwareDatabaseUtils.h"
#include "Services/ServiceNames.h"
#include "Services/DatabaseService.h"
#include "Services/SettingsService.h"
#include "Services/EventService.h"
#include "Services/RemoteService.h"
#include "Services/GUIService.h"
#include "Services/CryptService.h"
#include "NetworkTaskManager/NetworkTask.h"
#include "NetworkTaskManager/MemoryDataStream.h"
#include "NetworkTaskManager/NetworkTaskManager.h"

#include "TerminalService.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
TerminalService * TerminalService::instance(IApplication * aApplication)
{
	return static_cast<TerminalService *>(aApplication->getCore()->getService(CServices::TerminalService));
}

//---------------------------------------------------------------------------
TerminalService::TerminalService(IApplication * aApplication)
	: mDbUtils(nullptr),
	mEventService(nullptr),
	mApplication(aApplication),
	mClient(createWatchServiceClient(CWatchService::Modules::PaymentProcessor, IWatchServiceClient::MainThread))
{
	setLog(mApplication->getLog());

	mApplication = aApplication;

	mEventService = mApplication->getCore()->getEventService();
	mEventService->subscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event &)));
}

//---------------------------------------------------------------------------
TerminalService::~TerminalService()
{
}

//---------------------------------------------------------------------------
bool TerminalService::initialize()
{
	DatabaseService * dbService = DatabaseService::instance(mApplication);

	mDbUtils = dbService->getDatabaseUtils<IHardwareDatabaseUtils>();

	// Добавляем устройство "терминал" в БД.
	if (!mDbUtils->hasDevice(PPSDK::CDatabaseConstants::Devices::Terminal))
	{
		if (!mDbUtils->addDevice(PPSDK::CDatabaseConstants::Devices::Terminal))
		{
			toLog( LogLevel::Error, "Failed to add a record to database.");
			return false;
		}
	}

	mSettings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>()->getCommonSettings();

	PPSDK::IDeviceService * deviceService = mApplication->getCore()->getDeviceService();
	connect(deviceService, SIGNAL(deviceStatusChanged(const QString &, SDK::Driver::EWarningLevel::Enum, const QString &, int)),
		SLOT(onDeviceStatusChanged(const QString &, SDK::Driver::EWarningLevel::Enum, const QString &, int)));

	connect(deviceService, SIGNAL(configurationUpdated()), SLOT(onHardwareConfigUpdated()));

	// Проверяем наличие ключей.
	auto key = CryptService::instance(mApplication)->getKey(0);

	if (!key.isValid)
	{
		setTerminalError(PPSDK::ETerminalError::KeyError, true);
	}
	else
	{
		checkConfigsIntegrity();
	}

	// Обновляем количество запусков ПО.
	int restartCount = getRestartCount();

	restartCount++;

	setRestartCount(restartCount);

	// Проверяем состояние блокировки терминала
	if (mDbUtils->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::DisabledParam).isNull())
	{
		writeLockStatus(false);
	}

	mDbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::Configuration, 
		mApplication->getSettings().value("common/configuration"));

	mDbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::OperationSystem, 
		ISysUtils::getOSVersionInfo());

	// Проверяем параметры командной строки.
	if (mClient->start() || mApplication->getSettings().value("common/standalone").toBool())
	{
		return true;
	}
	else
	{
		toLog(LogLevel::Error, "Guard service module is not available.");
		return false;
	}
}

//------------------------------------------------------------------------------
void TerminalService::finishInitialize()
{
	auto guiService = GUIService::instance(mApplication);

	if (guiService)
	{
		QStringList screenList;
		QRect resolution;

		do
		{
			resolution = guiService->getScreenSize(screenList.size());

			if (!resolution.isEmpty())
			{
				screenList << QString("{\"width\":%1,\"height\":%2}").arg(resolution.width()).arg(resolution.height());
			}
		} while (!resolution.isEmpty());

		mDbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, QString(PPSDK::CDatabaseConstants::Parameters::DisplayResolution),
			QString("[%1]").arg(screenList.join(",")));
	}

	// Получаем информацию о дефолтном ключе
	PPSDK::TerminalSettings * terminalSettings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();
	PPSDK::SKeySettings key = terminalSettings->getKeys().value(0);

	// Устанавливаем User-Agent (Имя_ПО + Версия_ПО + код_диллера + код_точки + код_оператора)
	QString userAgent = Cyberplat::Application + " " + Cyberplat::getVersion();
	if (key.isValid)
	{
		userAgent.append(" SD:" + key.sd + " AP:" + key.ap + " OP:" + key.op);
	}

	PPSDK::INetworkService * networkService = mApplication->getCore()->getNetworkService();
	networkService->setUserAgent(userAgent);
}

//---------------------------------------------------------------------------
bool TerminalService::canShutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool TerminalService::shutdown()
{
	// Показываем экран блокировки WatchService.
	mClient->resetState();
	mClient->stop();

	return true;
}

//---------------------------------------------------------------------------
QString TerminalService::getName() const
{
	return CServices::TerminalService;
}

//---------------------------------------------------------------------------
const QSet<QString> & TerminalService::getRequiredServices() const
{
	static QSet<QString> required = QSet<QString>()
		<< CServices::DatabaseService
		<< CServices::EventService
		<< CServices::SettingsService
		<< CServices::DeviceService
		<< CServices::CryptService;

	return required;
}

//---------------------------------------------------------------------------
QVariantMap TerminalService::getParameters() const
{
	QVariantMap parameters;

	parameters[PPSDK::CServiceParameters::Terminal::RestartCount] = getRestartCount();

	return parameters;
}

//---------------------------------------------------------------------------
bool TerminalService::isLocked() const
{
	return isDisabled();
}

//---------------------------------------------------------------------------
void TerminalService::setLock(bool aIsLocked)
{
	// Отключаем интерфейс.
	auto guiService = GUIService::instance(mApplication);

	if (guiService)
	{
		guiService->disable(aIsLocked);
	}

	writeLockStatus(aIsLocked);
}

//---------------------------------------------------------------------------
void TerminalService::resetParameters(const QSet<QString> & aParameters)
{
	if (aParameters.contains(PPSDK::CServiceParameters::Terminal::RestartCount))
	{
		mDbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::LaunchCount, 0);
	}
}

//---------------------------------------------------------------------------
void TerminalService::writeLockStatus(bool aIsLocked)
{
	// Если разблокируем, то все критические ошибки превращаем в некритические (кроме ошибки БД)
	if (!aIsLocked)
	{
		foreach (auto key, mTerminalStatusHash.keys())
		{
			if (key != CServices::DatabaseService &&
				mTerminalStatusHash[key].getType() >= PPSDK::EEventType::Warning)
			{
				mTerminalStatusHash.remove(key);
			}
			else if (mTerminalStatusHash[key].getData().toString().contains("#alarm"))
			{
				PPSDK::Event e = mTerminalStatusHash[key];
				
				mTerminalStatusHash[key] = PPSDK::Event(e.getType(), e.getSender(), e.getData().toString().remove("#alarm"));
			}
		}

		// Сбрасываем "плохие" статусы при разблокировке
		TStatusCodes statuses = mDeviceErrorFlags.values(PPSDK::CDatabaseConstants::Devices::Terminal).toSet();

		foreach (auto status, statuses)
		{
			if (TerminalStatusCode::Specification[status].warningLevel >= SDK::Driver::EWarningLevel::Warning)
			{
				mDeviceErrorFlags.remove(PPSDK::CDatabaseConstants::Devices::Terminal, status);
			}
		}
	}

	updateTerminalStatus();

	// Проверка на "Уже записали статус в БД?", иначе при блокировке по ошибке БД мы зацикливаемся
	if (mLocked.is_initialized() && mLocked.get() == aIsLocked)
	{
		return;
	}
	else
	{
		mLocked = aIsLocked;

		mDbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::DisabledParam, mLocked.get());
	}
}

//---------------------------------------------------------------------------
bool TerminalService::isDisabled() const
{
	if (mLocked.is_initialized())
	{
		return mLocked.get();
	}
	
	mLocked = mDbUtils->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::DisabledParam).toBool();

	return mLocked.get();
}

//---------------------------------------------------------------------------
// TODO: перейти от эвентов к методам интерфейса ITerminalService.
void TerminalService::onEvent(const SDK::PaymentProcessor::Event & aEvent)
{
	PPSDK::EEventType::Enum eventType = static_cast<PPSDK::EEventType::Enum>(aEvent.getType());

	switch (eventType)
	{
		// Выключить терминал.
		case PPSDK::EEventType::TerminalLock:
		{
			setLock(true);
			break;
		}

		// Включить терминал.
		case PPSDK::EEventType::TerminalUnlock:
		{
			setLock(false);
			break;
		}

		case PPSDK::EEventType::RestoreConfiguration:
		{
			if (!RemoteService::instance(mApplication)->restoreConfiguration())
			{
				toLog(LogLevel::Error, "Failed to execute restore configuration command.");
			}

			break;
		}

		case PPSDK::EEventType::OK:
		case PPSDK::EEventType::Warning:
		case PPSDK::EEventType::Critical:
		{
			mTerminalStatusHash[aEvent.getSender()] = aEvent;

			if (aEvent.getSender() == CServices::DatabaseService)
			{
				setTerminalError(PPSDK::ETerminalError::DatabaseError, aEvent.getType() == PPSDK::EEventType::Critical);
			}
			else if (aEvent.getSender() == "AccountBalance")
			{
				setTerminalError(PPSDK::ETerminalError::AccountBalanceError, eventType != PPSDK::EEventType::OK);
			}
			else
			{
				updateTerminalStatus();
			}

			break;
		}
	}
}

//---------------------------------------------------------------------------
QPair<SDK::Driver::EWarningLevel::Enum, QString> TerminalService::getTerminalStatus() const
{
	QStringList resultMessage;
	SDK::Driver::EWarningLevel::Enum resultLevel = SDK::Driver::EWarningLevel::OK;

	auto convertStatus = [](PPSDK::EEventType::Enum aEventType) -> SDK::Driver::EWarningLevel::Enum {
		switch (aEventType)
		{
		case PPSDK::EEventType::Warning: return SDK::Driver::EWarningLevel::Warning;
		case PPSDK::EEventType::Critical: return SDK::Driver::EWarningLevel::Error;
		default: return SDK::Driver::EWarningLevel::OK;
		}
	};

	QMapIterator<QString, SDK::PaymentProcessor::Event> i(mTerminalStatusHash);
	while (i.hasNext())
	{
		i.next();

		auto status = convertStatus(static_cast<PPSDK::EEventType::Enum>(i.value().getType()));
		if (status > resultLevel)
		{
			resultLevel = status;
		}

		QString data = i.value().getData().toString();

		if (status != SDK::Driver::EWarningLevel::OK || 
			(!data.isEmpty() && data != "OK"))
		{
			resultMessage << QString("%1: %2").arg(i.value().getSender()).arg(i.value().getData().toString());
		}
	}

	TStatusCodes statuses = mDeviceErrorFlags.values(PPSDK::CDatabaseConstants::Devices::Terminal).toSet();

	if (statuses.isEmpty())
	{
		statuses << DeviceStatusCode::OK::OK;
	}

	foreach(int status, statuses)
	{
		resultMessage << TerminalStatusCode::Specification[status].translation;

		if (resultLevel < TerminalStatusCode::Specification[status].warningLevel)
		{
			resultLevel = TerminalStatusCode::Specification[status].warningLevel;
		}
	}

	if (isLocked())
	{
		resultMessage << "Locked";
		resultLevel = SDK::Driver::EWarningLevel::Error;
	}

	if (resultMessage.isEmpty())
	{
		resultMessage << "OK";
	}

	return QPair<SDK::Driver::EWarningLevel::Enum, QString>(resultLevel, resultMessage.join("\n"));
}

//---------------------------------------------------------------------------
void TerminalService::onDeviceStatusChanged(const QString & aConfigName, SDK::Driver::EWarningLevel::Enum aLevel, const QString & aDescription, int aStatus)
{
	Q_UNUSED(aDescription);

	namespace DeviceType = SDK::Driver::CComponents;

	QString deviceType = aConfigName.section('.', 2, 2);

	if (getAcceptorTypes().contains(deviceType) ||
		DeviceType::isPrinter(deviceType) ||
#ifdef TC_USE_TOKEN
		deviceType == DeviceType::Token ||
#endif
		deviceType == DeviceType::CardReader)
	{
		if (aLevel == SDK::Driver::EWarningLevel::Error || SDK::Driver::EStatus::Interface == aStatus)
		{
			// Device has entered error state
			mDeviceErrorFlags.insertMulti(aConfigName, aStatus);
		}
		else
		{
			// Device has exited error state
			mDeviceErrorFlags.remove(aConfigName);
		}
	}

	bool autoencashment = false;

	// Автоинкассация.
	if (deviceType == DeviceType::BillAcceptor && aStatus == SDK::Driver::ECashAcceptorStatus::StackerOpen && mSettings.autoEncachement)
	{
		// Запускаем автоинкасацию только тогда, когда нет ошибок у валидатора.
		QList<int> validatorErrorFlags = mDeviceErrorFlags.values(mDeviceErrorFlags.key(SDK::Driver::ECashAcceptorStatus::StackerOpen));

		validatorErrorFlags.removeAll(SDK::Driver::ECashAcceptorStatus::StackerOpen);

		autoencashment = validatorErrorFlags.isEmpty();
	}

	if (autoencashment)
	{
		EventService::instance(mApplication)->sendEvent(PPSDK::EEventType::Autoencashment, QVariantMap());
	}
	else
	{
		updateGUI();
	}
}

//---------------------------------------------------------------------------
void TerminalService::setTerminalError(PPSDK::ETerminalError::Enum aErrorType, bool aError)
{
	bool changed = false;

	if (aError)
	{
		if (!mDeviceErrorFlags.contains(PPSDK::CDatabaseConstants::Devices::Terminal, aErrorType))
		{
			mDeviceErrorFlags.insert(PPSDK::CDatabaseConstants::Devices::Terminal, aErrorType);

			changed = true;
		}
	}
	else if (mDeviceErrorFlags.contains(PPSDK::CDatabaseConstants::Devices::Terminal, aErrorType))
	{
		mDeviceErrorFlags.remove(PPSDK::CDatabaseConstants::Devices::Terminal, aErrorType);

		changed = true;
	}

	if (changed)
	{
		updateGUI();

		updateTerminalStatus();
	}
}

//---------------------------------------------------------------------------
void TerminalService::updateGUI()
{
	auto guiService = GUIService::instance(mApplication);

	if (guiService)
	{
		guiService->disable(!getFaultyDevices(true).isEmpty() || isLocked());
	}
}

//---------------------------------------------------------------------------
bool TerminalService::isTerminalError(PPSDK::ETerminalError::Enum aErrorType) const
{
	return mDeviceErrorFlags.values(PPSDK::CDatabaseConstants::Devices::Terminal).contains(aErrorType);
}

//---------------------------------------------------------------------------
void TerminalService::updateTerminalStatus()
{
	auto warningLevel2LogLevel = [](SDK::Driver::EWarningLevel::Enum aEventType) -> LogLevel::Enum {
		switch (aEventType)
		{
		case SDK::Driver::EWarningLevel::Warning: return LogLevel::Warning;
		case SDK::Driver::EWarningLevel::Error: return LogLevel::Error;
		default: return LogLevel::Normal;
		}
	};

	auto status = getTerminalStatus();

	if (mTerminalStatusCache != status)
	{
		toLog(warningLevel2LogLevel(status.first), QString("Terminal status: %1.")
			.arg(status.second.replace("\r", "").replace("\n", ";")));

		mDbUtils->addDeviceStatus(PPSDK::CDatabaseConstants::Devices::Terminal, status.first, status.second);
	}
}

//---------------------------------------------------------------------------
void TerminalService::checkConfigsIntegrity()
{
	// Проверяем корректность загруженных настроек.
	bool valid = true;

	foreach (const PPSDK::ISettingsAdapter * settings, SettingsService::instance(mApplication)->enumerateAdapters())
	{
		valid = valid && settings->isValid();
	}

	namespace DbConstants = PPSDK::CDatabaseConstants;

	if (!valid)
	{
		setTerminalError(PPSDK::ETerminalError::ConfigError, true);

		// Проверяем время с прошедшего обновления.
		QDateTime lastTryTime = mDbUtils->getDeviceParam(DbConstants::Devices::Terminal, DbConstants::Parameters::LastUpdateTime).toDateTime();
		QDateTime now = QDateTime::currentDateTime();

		if (!lastTryTime.isValid() || lastTryTime.addSecs(60 * ConfigRestoreInterval) <= now)
		{
			EventService::instance(mApplication)->sendEvent(PPSDK::EEventType::RestoreConfiguration, QVariant());

			mDbUtils->setDeviceParam(DbConstants::Devices::Terminal, DbConstants::Parameters::LastUpdateTime, now);
		}
		else
		{
			int retryTimeout = now.secsTo(lastTryTime.addSecs(60 * ConfigRestoreInterval));

			toLog(LogLevel::Warning, QString("Configuration restore failed: maximum attemps reached. Will try again in %1 sec.").arg(retryTimeout));

			// Запускаем таймер на повторение операции.
			QTimer::singleShot(retryTimeout * 1000, this, SLOT(checkConfigsIntegrity()));
		}
	}
}

//---------------------------------------------------------------------------
void TerminalService::sendFeedback(const QString & aSenderSubsystem, const QString & aMessage)
{
	// Отбрасываем дубли сообщений
	static QSet<QString> sentMessages;

	QString url = static_cast<PPSDK::TerminalSettings *>(mApplication->getCore()
		->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter))->getFeedbackURL();

	if (!url.isEmpty() && !sentMessages.contains(aMessage))
	{
		QByteArray sendBody;

		sendBody += "complain=1&offer=1&";
		sendBody += "PMVer=" + Cyberplat::getVersion().toUtf8().toPercentEncoding() + "&";

		auto key = CryptService::instance(mApplication)->getKey(0);
		sendBody += QString("SD=%1&").arg(key.sd) + QString("AP=%1&").arg(key.ap) + QString("OP=%1&").arg(key.op);

		sendBody += "Display=AxB&";
		sendBody += "WinVer=" + ISysUtils::getOSVersionInfo().toUtf8().toPercentEncoding() + "&";

		sendBody += "KKM=" + QString("%1 (%2) BankKey=%3")
			.arg("")
			.arg("")
			.arg(key.bankSerialNumber)
			.toUtf8().toPercentEncoding();
		sendBody += "&";

		sendBody += "subject=" + QString("TC3:%1").arg(aSenderSubsystem).toUtf8().toPercentEncoding() + "&";
		sendBody += "email=" + QString("help@cyberplat.com").toUtf8().toPercentEncoding() + "&";
		sendBody += "message=" + aMessage.toUtf8().toPercentEncoding() + "&";

		if (qint64 paymentId = mApplication->getCore()->getPaymentService()->getActivePayment())
		{
			sendBody += "pluslog=1&";
			QByteArray zipArray = "[{\"fields\":{";

			auto fields = mApplication->getCore()->getPaymentService()->getPaymentFields(paymentId);

			foreach (auto f, fields)
			{
				zipArray += QString("\"%1\":\"%2\",").arg(f.name).arg(f.value.toString()).toUtf8();
			}

			zipArray += "}}";

			double changeAmount = mApplication->getCore()->getPaymentService()->getChangeAmount();

			if (!qFuzzyIsNull(changeAmount) && changeAmount > 0.)
			{
				zipArray += QString(",{\"CHANGE_AMOUNT\":\"%1\"}").arg(changeAmount, 0, 'f', 2).toUtf8();
			}

			zipArray += "]";

			QByteArray gz;
			if (Packer::gzipCompress(zipArray, QString("payment_%1.json").arg(paymentId), gz))
			{
				sendBody += "pluslog_val=";
				sendBody += gz.toBase64().toPercentEncoding() + "&";
			}
		}

		NetworkTask * task = new NetworkTask();
		task->setUrl(url);
		task->setType(NetworkTask::Type::Post);
		task->setTimeout(60 * 1000);
		task->setDataStream(new MemoryDataStream());
		task->getRequestHeader().insert("Content-Type", "application/x-www-form-urlencoded");
		task->getDataStream()->write(sendBody);

		mApplication->getCore()->getNetworkService()->getNetworkTaskManager()->addTask(task);
		sentMessages.insert(aMessage);
	}
}

//---------------------------------------------------------------------------
void TerminalService::needUpdateConfigs()
{
	// Переименовываем config.xml, тем самым конфиги будут скачаны заново при следующем запуске.
	QString backupExt = QDateTime::currentDateTime().toString(".yyyy-MM-dd_hh-mm-ss") + "_backup";
	QString config = QString("%1%2%3").arg(mApplication->getUserDataPath()).arg(QDir::separator()).arg("config.xml");

	if (!QFile::rename(config, config + backupExt))
	{
		toLog(LogLevel::Error, "Failed to backup config.xml.");
	}
}

//---------------------------------------------------------------------------
QStringList TerminalService::getAcceptorTypes() const
{
	return QStringList()
		<< SDK::Driver::CComponents::BillAcceptor
		<< SDK::Driver::CComponents::CoinAcceptor;
}

//---------------------------------------------------------------------------
QStringList TerminalService::getDeviceNames() const
{
	return 
		QStringList(mApplication->getCore()->getDeviceService()->getConfigurations()) 
		<< PPSDK::CDatabaseConstants::Devices::Terminal;
}

//---------------------------------------------------------------------------
void TerminalService::onHardwareConfigUpdated()
{
	// Согласовываем список устройств в состоянии ошибки с текущим списком устройств.
	QStringList configNames = getDeviceNames();

	foreach (auto configName, mDeviceErrorFlags.keys())
	{
		if (!configNames.contains(configName))
		{
			mDeviceErrorFlags.remove(configName);
		}
	}
}

//---------------------------------------------------------------------------
IWatchServiceClient * TerminalService::getClient()
{
	return mClient.data();
}

//---------------------------------------------------------------------------
QMap<QString, int> TerminalService::getFaultyDevices(bool aActual) const
{
	namespace DeviceType = SDK::Driver::CComponents;

	auto result = mDeviceErrorFlags;

	if (aActual)
	{
		QStringList cashAcceptors = getAcceptorTypes();
		QStringList faultyCashDeviceNames;
		QStringList cashDeviceNames;

		foreach (auto name, result.keys())
		{
			if (cashAcceptors.contains(name.section('.', 2, 2)))
			{
				faultyCashDeviceNames << name;
			}
		}

		foreach(auto name, getDeviceNames())
		{
			if (cashAcceptors.contains(name.section('.', 2, 2)))
			{
				cashDeviceNames << name;
			}
		}

		bool hasValidatorError = cashDeviceNames.toSet() == faultyCashDeviceNames.toSet();

		foreach(const QString & device, result.keys())
		{
			QString deviceType = device.section('.', 2, 2);

			if ((cashAcceptors.contains(deviceType) && (!mSettings.blockOn(PPSDK::SCommonSettings::ValidatorError) || !hasValidatorError)) ||
				(DeviceType::isPrinter(deviceType) && !mSettings.blockOn(PPSDK::SCommonSettings::PrinterError)) ||
#ifndef TC_USE_TOKEN
				(deviceType == DeviceType::Token) ||
#endif
				(deviceType == DeviceType::CardReader && !mSettings.blockOn(PPSDK::SCommonSettings::CardReaderError)))
			{
				result.remove(device);
			}
		}
	}

	return result;
}

//---------------------------------------------------------------------------
void TerminalService::setRestartCount(int aCount)
{
	QDate lastStartDate = mDbUtils->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::LastStartDate).toDate();
	QDate currentDate = QDate::currentDate();

	if (lastStartDate != currentDate)
	{
		mDbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::LastStartDate, currentDate);
		aCount = 0;
	}

	mDbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::LaunchCount, aCount);
}

//---------------------------------------------------------------------------
int TerminalService::getRestartCount() const
{
	int count = mDbUtils->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::LaunchCount).toInt();
	QDate lastStartDate = mDbUtils->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal, PPSDK::CDatabaseConstants::Parameters::LastStartDate).toDate();

	if (lastStartDate != QDate::currentDate())
	{
		count = 0;
	}

	return count;
}

//---------------------------------------------------------------------------
