/* @file Менеджер, загружающий клиенты мониторинга. */

// Stl
#include <functional> 

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QMutexLocker>
#include <QtCore/QFileSystemWatcher>
#include <Common/QtHeadersEnd.h>

// Modules
#include <WatchServiceClient/Constants.h>

// SDK
#include <SDK/Drivers/Components.h>
#include <SDK/PaymentProcessor/Core/DatabaseConstants.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Project
#include "DatabaseUtils/IHardwareDatabaseUtils.h"

#include "Services/ServiceNames.h"
#include "Services/DatabaseService.h"
#include "Services/TerminalService.h"
#include "Services/PluginService.h"
#include "Services/EventService.h"
#include "Services/PaymentService.h"
#include "Services/RemoteService.h"
#include "Services/SettingsService.h"
#include "Services/CryptService.h"
#include "Services/DeviceService.h"

namespace CRemoteService
{
	// Лог сервиса
	const QString LogName = "Monitoring";

	// Файл конфигурации сервиса
	const QString ConfigFileName = "/update/update_commands.ini";

	// Номер последней команды обновления, зарегистрированной у модуля обновления.
	const char LastMonitoringCommand[] = "last_monitoring_command";

	// Список отложенных команд перезагрузки и выключения. Их статусы будут обновлены при следующем включении клиента.
	const char QueuedRebootCommands[] = "queued_reboot_commands";

	// Номер текущей команды перегенерации ключей 
	const char GenKeyCommandId[] = "gen_key_commands";

	// Разделитель значений в свойстве QueuedRebootCommands.
	const char QueuedRebootCommandDelimeter[] = "|";

	const int UpdateReportCheckTimeout = 5 * 1000;
	const int UpdateReportFirstCheckInterval = 1 * 60 * 1000;
	const int UpdateReportCheckInterval = 10 * 60 * 1000;
	const int CommandCheckInterval = 1 * 60 * 1000;
	const int MaxUpdateCommandLifetime = 60 * 30;

	const int GeneratedKeyId = 100;

	const char DateTimeFormat[] = "yyyy.MM.dd hh:mm:ss";
}

//---------------------------------------------------------------------------
struct CommandReport
{
	QString filePath;

	int ID;
	SDK::PaymentProcessor::IRemoteService::EStatus status;
	QDateTime lastUpdate;
	QVariant description;
	QVariant progress;

	CommandReport(const QString & aFilePath) : filePath(aFilePath)
	{
		ID = 0;
		status = SDK::PaymentProcessor::IRemoteService::OK;
	}

	/// Удалить файл отчета
	void remove()
	{
		QFile::remove(filePath);
	}

	/// Проверк что команда ещё выполняется, а не "подвисла"
	bool isAlive()
	{
		return lastUpdate.addSecs(60 * 10) > QDateTime::currentDateTime();
	}
};

//---------------------------------------------------------------------------
RemoteService * RemoteService::instance(IApplication * aApplication)
{
	return static_cast<RemoteService *>(aApplication->getCore()->getService(CServices::RemoteService));
}

//---------------------------------------------------------------------------
RemoteService::RemoteService(IApplication * aApplication)
	: ILogable(CRemoteService::LogName),
	  mApplication(aApplication),
	  mDatabase(0),
	  mGenerateKeyCommand(0),
	  mSettings(aApplication->getWorkingDirectory() + CRemoteService::ConfigFileName, QSettings::IniFormat),
	  mCommandMutex(QMutex::Recursive)
{
	// Сооздаем 5сек таймер отложенной проверки состояния файлов отчета
	mCheckUpdateReportsTimer.setSingleShot(true);
	mCheckUpdateReportsTimer.setInterval(CRemoteService::UpdateReportCheckTimeout);
	connect(&mCheckUpdateReportsTimer, SIGNAL(timeout()), this, SLOT(onCheckUpdateReports()));
}

//---------------------------------------------------------------------------
RemoteService::~RemoteService()
{
}

//---------------------------------------------------------------------------
bool RemoteService::initialize()
{
	mDatabase = DatabaseService::instance(mApplication)->getDatabaseUtils<IHardwareDatabaseUtils>();

	QVariant lastCommand = mDatabase->getDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal, CRemoteService::LastMonitoringCommand);
	if (!lastCommand.isValid())
	{
		mLastCommand = 0;
	}
	else
	{
		mLastCommand = lastCommand.toInt();
	}

	connect(PaymentService::instance(mApplication), SIGNAL(paymentCommandComplete(int, EPaymentCommandResult::Enum)), 
		SLOT(onPaymentCommandComplete(int, EPaymentCommandResult::Enum)), Qt::QueuedConnection);

	QStringList remotes =
		PluginService::instance(mApplication)->getPluginLoader()->getPluginList(QRegExp(QString("%1\\.%2\\..*").arg(PPSDK::Application, PPSDK::CComponents::RemoteClient)));

	foreach (const QString & path, remotes)
	{
		SDK::Plugin::IPlugin * plugin = PluginService::instance(mApplication)->getPluginLoader()->createPlugin(path);
		if (plugin)
		{
			PPSDK::IRemoteClient * client = dynamic_cast<PPSDK::IRemoteClient *>(plugin);
			if (client)
			{
				mMonitoringClients << client;
			}
			else
			{
				PluginService::instance(mApplication)->getPluginLoader()->destroyPlugin(plugin);
			}
		}
	}

	auto deviceService = DeviceService::instance(mApplication);

	connect(deviceService, SIGNAL(configurationUpdated()), this, SLOT(onDeviceConfigurationUpdated()));
	connect(deviceService, SIGNAL(deviceStatusChanged(const QString &, SDK::Driver::EWarningLevel::Enum, const QString &, int)),
		this, SLOT(onDeviceStatusChanged(const QString &)), Qt::QueuedConnection);

	return true;
}

//------------------------------------------------------------------------------
void RemoteService::onDeviceConfigurationUpdated()
{
	foreach (auto client, mMonitoringClients)
	{
		client->useCapability(PPSDK::IRemoteClient::DeviceConfigurationUpdated);
	}
}

//------------------------------------------------------------------------------
void RemoteService::finishInitialize()
{
	restoreCommandQueue();

	foreach (auto client, mMonitoringClients)
	{
		client->enable();
	}

	if (mDatabase)
	{
		bool ok = true;
		mGenerateKeyCommand = mDatabase->getDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal, CRemoteService::GenKeyCommandId).toInt(&ok);
		mGenerateKeyCommand = ok ? mGenerateKeyCommand : 0;

		QTimer::singleShot(CRemoteService::UpdateReportFirstCheckInterval, this, SLOT(onCheckUpdateReports()));
		QTimer::singleShot(CRemoteService::CommandCheckInterval, this, SLOT(onCheckQueuedRebootCommands()));

		QDir(mApplication->getWorkingDirectory()).mkpath("update");

		restartUpdateWatcher();

		// запускаем независимый таймер, отслеживающий состояние команд обновления
		startTimer(CRemoteService::UpdateReportCheckInterval);
	}

	if (!mScreenShotsCommands.isEmpty())
	{
		QTimer::singleShot(CRemoteService::CommandCheckInterval, this, SLOT(doScreenshotCommand()));
	}
}

//---------------------------------------------------------------------------
bool RemoteService::canShutdown()
{
	return true;
}

//---------------------------------------------------------------------------
bool RemoteService::shutdown()
{
	saveCommandQueue();

	if (mGenerateKeyFuture.isRunning())
	{
		mGenerateKeyFuture.waitForFinished();
	}

	disconnect(PaymentService::instance(mApplication), SIGNAL(paymentCommandComplete(int, EPaymentCommandResult::Enum)), 
		this, SLOT(onPaymentCommandComplete(int, EPaymentCommandResult::Enum)));

	while (!mMonitoringClients.isEmpty())
	{
		mMonitoringClients.first()->disable();

		PluginService::instance(mApplication)->getPluginLoader()->destroyPlugin(
			dynamic_cast<SDK::Plugin::IPlugin *>(mMonitoringClients.first()));

		mMonitoringClients.takeFirst();
	}

	mCheckUpdateReportsTimer.stop();
	mDatabase->setDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal,
		CRemoteService::QueuedRebootCommands, mQueuedRebootCommands.join(CRemoteService::QueuedRebootCommandDelimeter));

	return true;
}

//---------------------------------------------------------------------------
QString RemoteService::getName() const
{
	return CServices::RemoteService;
}

//---------------------------------------------------------------------------
const QSet<QString> & RemoteService::getRequiredServices() const
{
	// TODO: пересмотреть зависимости
	static QSet<QString> requiredServices = QSet<QString>()
		<< CServices::SettingsService
		<< CServices::EventService
		<< CServices::PluginService
		<< CServices::DatabaseService
		<< CServices::PaymentService
		<< CServices::DeviceService;

	return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap RemoteService::getParameters() const
{
	return QVariantMap();
}

//---------------------------------------------------------------------------
void RemoteService::resetParameters(const QSet<QString> &)
{
}

//---------------------------------------------------------------------------
int RemoteService::increaseLastCommandID()
{
	++mLastCommand;

	mDatabase->setDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal, CRemoteService::LastMonitoringCommand, mLastCommand);

	return mLastCommand;
}

//---------------------------------------------------------------------------
int RemoteService::executeCommand(PPSDK::EEventType::Enum aEvent)
{
	QMutexLocker lock(&mCommandMutex);

	int command = increaseLastCommandID();

	QMetaObject::invokeMethod(this, "doExecuteCommand", Qt::QueuedConnection, Q_ARG(int, command), Q_ARG(int, aEvent));

	return command;
}

//---------------------------------------------------------------------------
void RemoteService::doExecuteCommand(int aComandId, int aEvent)
{
	if ((aEvent == PPSDK::EEventType::Reboot) || (aEvent == PPSDK::EEventType::Shutdown))
	{
		if (!allowRestart())
		{
			emit commandStatusChanged(aComandId, Error, QVariantMap());

			return;
		}

		mQueuedRebootCommands << QString::number(aComandId);
	}
	else if (aEvent == PPSDK::EEventType::Restart)
	{
		mQueuedRebootCommands << QString::number(aComandId);
	}
	else
	{
		emit commandStatusChanged(aComandId, OK, QVariantMap());
	}

	EventService::instance(mApplication)->sendEvent(SDK::PaymentProcessor::Event(aEvent));
}

//---------------------------------------------------------------------------
int RemoteService::registerLockCommand()
{
	return executeCommand(PPSDK::EEventType::TerminalLock);
}

//---------------------------------------------------------------------------
int RemoteService::registerUnlockCommand()
{
	return executeCommand(PPSDK::EEventType::TerminalUnlock);
}

//---------------------------------------------------------------------------
int RemoteService::registerRebootCommand()
{
	return executeCommand(PPSDK::EEventType::Reboot);
}

//---------------------------------------------------------------------------
int RemoteService::registerRestartCommand()
{
	return executeCommand(PPSDK::EEventType::Restart);
}

//---------------------------------------------------------------------------
int RemoteService::registerShutdownCommand()
{
	return executeCommand(PPSDK::EEventType::Shutdown);
}

//---------------------------------------------------------------------------
int RemoteService::registerAnyCommand()
{
	return increaseLastCommandID();
}

//---------------------------------------------------------------------------
int RemoteService::registerPaymentCommand(EPaymentOperation aOperation, const QString & aInitialSession, const QVariantMap & aParameters)
{
	QMutexLocker lock(&mCommandMutex);

	if (mGenerateKeyCommand)
	{
		return 0;
	}

	int paymentCommand = aOperation == Remove ?
		PaymentService::instance(mApplication)->registerRemovePaymentCommand(aInitialSession) :
		PaymentService::instance(mApplication)->registerForcePaymentCommand(aInitialSession, aParameters);

	if (!paymentCommand)
	{
		return 0;
	}
	else
	{
		int command = increaseLastCommandID();

		mPaymentCommands.insert(paymentCommand, command);

		return command;
	}
}

//---------------------------------------------------------------------------
void RemoteService::onPaymentCommandComplete(int aID, EPaymentCommandResult::Enum aError)
{
	int status = Error;
	switch(aError)
	{
	case EPaymentCommandResult::OK:
		status = OK;
		break;

	case EPaymentCommandResult::NotFound:
		status = PaymentNotFound;
		break;

	default:
		status = Error;
	}

	emit commandStatusChanged(mPaymentCommands[aID], status, QVariantMap());

	mPaymentCommands.remove(aID);
}

//---------------------------------------------------------------------------
bool RemoteService::allowUpdateCommand()
{
	return mUpdateCommands.isEmpty() &&
		   mGenerateKeyCommand == 0 &&
		   mQueuedRebootCommands.isEmpty();
}

//---------------------------------------------------------------------------
bool RemoteService::allowRestart()
{
	if (!mUpdateCommands.isEmpty())
	{
		// ускорение проверки статуса команды получения конфигурации
		onCheckUpdateReports();
	}

	foreach (auto command, mUpdateCommands.values())
	{
		if (command.type == FirmwareUpload)
		{
			toLog(LogLevel::Error, "Deny restart/shutdown because FirmwareUpload processed.");

			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------
int RemoteService::registerUpdateCommand(EUpdateType aType, const QUrl & aConfigUrl, const QUrl & aUpdateUrl, const QString & aComponents)
{
	UpdateCommand command;

	command.ID = increaseLastCommandID();
	command.status = IRemoteService::Waiting;
	command.type = aType;
	command.configUrl = aConfigUrl;
	command.updateUrl = aUpdateUrl;

	if (!aComponents.trimmed().isEmpty())
	{
		command.parameters = aComponents.trimmed().split("#");
	}

	if (!mUpdateCommands.isEmpty())
	{
		// ускорение проверки статуса команды получения конфигурации
		onCheckUpdateReports();
	}

	if (!allowUpdateCommand())
	{
		QMutexLocker lock(&mCommandMutex);

		toLog(LogLevel::Normal, QString("Update command added to the queue. ID:%1 type:%2 url:%3 url2:%4 parameters:%5.")
			.arg(command.ID).arg(command.type).arg(command.configUrl.toString())
			.arg(command.updateUrl.toString()).arg(command.parameters.join("#")));

		mUpdateCommands.insert(command.ID, command);
		
		saveCommandQueue();

		return command.ID;
	}

	return startUpdateCommand(command);
}

//---------------------------------------------------------------------------
int RemoteService::startUpdateCommand(UpdateCommand aCommand)
{
	auto appInfo = mApplication->getAppInfo();

	// Сериализуем настройки прокси.
	auto settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();

	QString commandParams = QString("--server \"%1\" --version \"%2\" --application %3 --conf %4 --id %5 --point %6 --accept-keys %7")
		.arg(aCommand.configUrl.toString()).arg(appInfo.version).arg(appInfo.appName).arg(appInfo.configuration).arg(aCommand.ID)
		.arg(settings->getKeys()[0].ap).arg(settings->getKeys()[0].bankSerialNumber);

	auto proxy = settings->getConnection().proxy;
	if (proxy.type() != QNetworkProxy::NoProxy)
	{
		commandParams += QString(" --proxy %1:%2:%3:%4:%5").arg(proxy.hostName()).arg(proxy.port()).arg(proxy.user()).arg(proxy.password()).arg(proxy.type());
	}

	switch (aCommand.type)
	{
	case Configuration:
		commandParams += " --command config";
		if (!aCommand.parameters.isEmpty())
		{
			commandParams += QString(" --md5 %1").arg(aCommand.parameters.first());
		}
		break;

	case Update:
		commandParams += QString(" --command update --update-url \"%1\"").arg(aCommand.updateUrl.toString());
		if (!aCommand.parameters.isEmpty())
		{
			commandParams += QString(" --components \"%1\"").arg(aCommand.parameters.join("#"));
		}
		break;

	case UserPack:
		commandParams += " --command userpack";
		if (!aCommand.parameters.isEmpty())
		{
			commandParams += QString(" --md5 %1").arg(aCommand.parameters.first());
		}
		break;

	case AdUpdate:
		commandParams += " --command userpack --no-restart true --destination-subdir ad";
		if (!aCommand.parameters.isEmpty())
		{
			commandParams += QString(" --md5 %1").arg(aCommand.parameters.first());
		}
		break;

	case FirmwareDownload:
		if (!aCommand.parameters.isEmpty())
		{
			commandParams += QString(" --command userpack --no-restart true --destination-subdir update/%1").arg(aCommand.parameters.at(1));
			commandParams += QString(" --md5 %1").arg(aCommand.parameters.at(0));
		}
		break;

	case CheckIntegrity:
		commandParams += " --command integrity";
		break;

	default:
		toLog(LogLevel::Error, QString("Unknown update command type: %1.").arg(aCommand.type));
		return 0;
	}

	aCommand.status = Executing;
	mUpdateCommands.insert(aCommand.ID, aCommand);

	TerminalService::instance(mApplication)->getClient()->subscribeOnModuleClosed(this);
	TerminalService::instance(mApplication)->getClient()->startModule(CWatchService::Modules::Updater, commandParams);

	emit commandStatusChanged(aCommand.ID, aCommand.status, QVariantMap());

	saveCommandQueue();

	return aCommand.ID;
}

//---------------------------------------------------------------------------
void RemoteService::doScreenshotCommand()
{
	while (!mScreenShotsCommands.isEmpty())
	{
		int command = mScreenShotsCommands.takeFirst();

		QVariantList value;
		foreach (auto image, mApplication->getScreenshot())
		{
			value.push_back(image);
		}

		QVariantMap parameters;
		parameters.insert(PPSDK::CMonitoringService::CommandParameters::Screenshots, value);

		emit commandStatusChanged(command, OK, parameters);
	}
}

//---------------------------------------------------------------------------
int RemoteService::registerScreenshotCommand()
{
	QMutexLocker lock(&mCommandMutex);

	int command = increaseLastCommandID();

	mScreenShotsCommands.push_back(command);
	QTimer::singleShot(100, this, SLOT(doScreenshotCommand()));

	return command;
}

//---------------------------------------------------------------------------
int RemoteService::registerGenerateKeyCommand(const QString & aLogin, const QString & aPassword)
{
	QMutexLocker lock(&mCommandMutex);

	if (mGenerateKeyCommand == 0)
	{
		mGenerateKeyCommand = increaseLastCommandID();

		mGenerateKeyFuture = QtConcurrent::run(std::tr1::bind(doGenerateKeyCommand, this, aLogin, aPassword));

		return mGenerateKeyCommand;
	}

	return 0;
}

//---------------------------------------------------------------------------
void RemoteService::doGenerateKeyCommand(RemoteService * aService, const QString & aLogin, const QString & aPassword)
{
	auto terminalSettings = static_cast<PPSDK::TerminalSettings *>(aService->mApplication->getCore()->
		getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));
	auto cryptoService = aService->mApplication->getCore()->getCryptService();

	QString url(terminalSettings->getKeygenURL());
	QString SD, AP, OP;
	
	int error = cryptoService->generateKey(CRemoteService::GeneratedKeyId, aLogin, aPassword, url, SD, AP, OP);
	if (error)
	{
		aService->toLog(LogLevel::Error, QString("GENKEY: Error generate key: %1.").arg(error));
	}
	else
	{
		error = !cryptoService->saveKey();
		if (error)
		{
			aService->toLog(LogLevel::Error, "GENKEY: Failed to save new key.");
		}
	}

	aService->commandStatusChanged(aService->mGenerateKeyCommand, error ? Error : OK, QVariantMap());

	aService->mGenerateKeyCommand = error ? 0 : aService->mGenerateKeyCommand;

	if (!error)
	{
		aService->toLog(LogLevel::Normal, QString("GENKEY: New key %1 generated. Wait for send command status to server.")
			.arg(CRemoteService::GeneratedKeyId));

		aService->mDatabase->setDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal, 
			CRemoteService::GenKeyCommandId, aService->mGenerateKeyCommand);
		aService->mApplication->getCore()->getSettingsService()->saveConfiguration();
	}
}

//---------------------------------------------------------------------------
void RemoteService::commandStatusSent(int aCommandId, int aStatus)
{
	if (mGenerateKeyCommand &&
		aCommandId == mGenerateKeyCommand &&
		aStatus == OK)
	{
		mGenerateKeyCommand = 0;

		if (mApplication->getCore()->getCryptService()->replaceKeys(CRemoteService::GeneratedKeyId, 0))
		{
			mApplication->getCore()->getSettingsService()->saveConfiguration();

			mDatabase->setDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal, CRemoteService::GenKeyCommandId, "");

			toLog(LogLevel::Normal, "GENKEY: Sucessful swap old and new crypto key.");
		}
		else
		{
			toLog(LogLevel::Error, "GENKEY: Error swap old and new crypto key. Terminal will be restarted.");

			executeCommand(PPSDK::EEventType::Reboot);
		}
	}
}

//---------------------------------------------------------------------------
void RemoteService::updateContent()
{
	foreach(auto client, mMonitoringClients)
	{
		client->useCapability(PPSDK::IRemoteClient::UpdateContent);
	}
}

//---------------------------------------------------------------------------
void RemoteService::sendHeartbeat()
{
	foreach(auto client, mMonitoringClients)
	{
		client->useCapability(PPSDK::IRemoteClient::SendHeartbeat);
	}
}

//---------------------------------------------------------------------------
void RemoteService::onCheckQueuedRebootCommands()
{
	QStringList ids = mDatabase->getDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal,
		CRemoteService::QueuedRebootCommands).toString().split(CRemoteService::QueuedRebootCommandDelimeter, QString::SkipEmptyParts);

	foreach (const QString & id, ids)
	{
		emit commandStatusChanged(id.toInt(), OK, QVariantMap());
	}

	mDatabase->setDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal, CRemoteService::QueuedRebootCommands, QString());
}

//---------------------------------------------------------------------------
void RemoteService::onUpdateDirChanged()
{
	mCheckUpdateReportsTimer.stop();

	toLog(LogLevel::Normal, "Directory 'update' changed.");

	restartUpdateWatcher(dynamic_cast<QFileSystemWatcher *>(sender()));

	// Таймаут что бы этот обработчик, вызыванный много раз в течении короткого времени, не запускал долгую процедуру проверки файлов отчетов.
	mCheckUpdateReportsTimer.start();
}

//---------------------------------------------------------------------------
QList<CommandReport> getReports(const QString & aReportsPath)
{
	QList<CommandReport> result;

	foreach (auto reportFileName, QDir(aReportsPath, "*.rpt").entryList(QDir::Files))
	{
		CommandReport r(aReportsPath + QDir::separator() + reportFileName);
		QSettings report(r.filePath, QSettings::IniFormat);

		QVariant idVar = report.value("id");
		r.ID = idVar.isValid() ? idVar.toInt() : reportFileName.mid(reportFileName.indexOf('_') + 1, reportFileName.indexOf('.') - reportFileName.indexOf('_') - 1).toInt();

		r.status = static_cast<SDK::PaymentProcessor::IRemoteService::EStatus>(report.value("status").toInt());
		r.lastUpdate = QDateTime::fromString(report.value("last_update").toString(), CRemoteService::DateTimeFormat);
		r.description = report.value("status_desc");
		r.progress = report.value("progress");

		result << r;
	}
	
	return result;
}

//---------------------------------------------------------------------------
int RemoteService::checkUpdateReports()
{
	int removedCount = 0;

	// Проверка репортов команд
	foreach(auto report, getReports(mApplication->getWorkingDirectory() + "/update/"))
	{
		QVariantMap parameters;

		if (report.description.isValid())
		{
			parameters.insert(SDK::PaymentProcessor::CMonitoringService::CommandParameters::Description, report.description);
		}

		switch (report.status)
		{
		case OK:
		{
			parameters.insert(SDK::PaymentProcessor::CMonitoringService::CommandParameters::Progress, "100");

			// Start firmware upload?
			if (checkFirmwareUpload(report.ID))
			{
				report.remove();
				removedCount++;

				parameters.insert(SDK::PaymentProcessor::CMonitoringService::CommandParameters::Progress, "50");
				emit commandStatusChanged(report.ID, Executing, parameters);
				break;
			}
		}

		case Error:
		{
			updateCommandFinish(report.ID, report.status, parameters);

			report.remove();
			removedCount++;

			break;
		}

		default:
		{
			if (report.progress.isValid())
			{
				parameters.insert(SDK::PaymentProcessor::CMonitoringService::CommandParameters::Progress, report.progress);
			}

			if (report.isAlive())
			{
				// Обновляем время изменения команды
				if (mUpdateCommands.contains(report.ID))
				{
					QMutexLocker lock(&mCommandMutex);
					mUpdateCommands[report.ID].lastUpdate = report.lastUpdate;
					saveCommandQueue();
				}

				emit commandStatusChanged(report.ID, Executing, parameters);
			}
			else
			{
				updateCommandFinish(report.ID, Error, parameters);

				report.remove();
				removedCount++;
			}

			break;
		}
		}
	}

	return removedCount;
}

//---------------------------------------------------------------------------
void RemoteService::onCheckUpdateReports()
{
	checkUpdateReports();
	checkCommandsLifetime();
}

//---------------------------------------------------------------------------
int RemoteService::checkCommandsLifetime()
{
	int removed = 0;

	foreach(auto cmd, mUpdateCommands.values())
	{
		if (cmd.lastUpdate.addSecs(CRemoteService::MaxUpdateCommandLifetime) < QDateTime::currentDateTime())
		{
			toLog(LogLevel::Error, QString("Command %1 lifetime has ended. (%2)").arg(cmd.ID).arg(cmd.configUrl.toString()));

			QMutexLocker lock(&mCommandMutex);
			mUpdateCommands.remove(cmd.ID);
			saveCommandQueue();
			removed++;

			emit commandStatusChanged(cmd.ID, Error, QVariantMap());
		}
	}

	return removed;
}

//---------------------------------------------------------------------------
bool RemoteService::checkFirmwareUpload(int aCommandID)
{
	if (!mUpdateCommands.contains(aCommandID))
	{
		return false;
	}

	auto command = mUpdateCommands.value(aCommandID);

	if (command.type != IRemoteService::FirmwareDownload)
	{
		return false;
	}

	command.type = IRemoteService::FirmwareUpload;
	command.status = IRemoteService::Executing;
	mUpdateCommands.insert(aCommandID, command);
	saveCommandQueue();

	toLog(LogLevel::Normal, QString("Complete download firmware. Restart command %1 for UPLOAD.").arg(aCommandID));

	EventService::instance(mApplication)->sendEvent(PPSDK::Event(PPSDK::EEventType::Restart, "updater", QString("-start_scenario=FirmwareUpload")));
	
	return true;
}

//---------------------------------------------------------------------------
bool RemoteService::restoreConfiguration()
{
	bool result = false;

	foreach (PPSDK::IRemoteClient * client, mMonitoringClients)
	{
		if (client->getCapabilities() & PPSDK::IRemoteClient::RestoreConfiguration)
		{
			result = client->useCapability(PPSDK::IRemoteClient::RestoreConfiguration);
		}
	}

	return result;
}

//---------------------------------------------------------------------------
void RemoteService::saveCommandQueue()
{
	mSettings.clear();

	foreach (auto command, mUpdateCommands)
	{
		mSettings.beginGroup(QString("cmd_%1").arg(command.ID));

		mSettings.setValue("id", command.ID);
		mSettings.setValue("configUrl", command.configUrl.toString());
		mSettings.setValue("updateUrl", command.updateUrl.toString());
		mSettings.setValue("type", command.type);
		mSettings.setValue("parameters", command.parameters.join("#"));
		mSettings.setValue("status", command.status);
		mSettings.setValue("lastUpdate", command.lastUpdate.toString(CRemoteService::DateTimeFormat));

		mSettings.endGroup();
	}

	QStringList screenShotsCommands;
	foreach(int cmd, mScreenShotsCommands)
	{
		screenShotsCommands << QString::number(cmd);
	}
	
	mSettings.setValue("common/screenshot", screenShotsCommands.join(";"));

	mSettings.sync();
}

//---------------------------------------------------------------------------
void RemoteService::restoreCommandQueue()
{
	mSettings.sync();

	foreach (auto group, mSettings.childGroups())
	{
		UpdateCommand cmd;

		mSettings.beginGroup(group);

		cmd.ID = mSettings.value("id").toInt();
		cmd.configUrl = mSettings.value("configUrl").toUrl();
		cmd.updateUrl = mSettings.value("updateUrl").toUrl();
		cmd.type = static_cast<EUpdateType>(mSettings.value("type").toInt());
		cmd.status = static_cast<EStatus>(mSettings.value("status").toInt());
		cmd.parameters = mSettings.value("parameters").toString().split("#");
		cmd.lastUpdate = QDateTime::fromString(mSettings.value("lastUpdate").toString(), CRemoteService::DateTimeFormat);

		// Если метка времени до сих пор не ипользовалась, заполняем её текущим временем.
		if (cmd.lastUpdate.isNull() || !cmd.lastUpdate.isValid())
		{
			cmd.lastUpdate = QDateTime::currentDateTime();
			mSettings.setValue("lastUpdate", cmd.lastUpdate.toString(CRemoteService::DateTimeFormat));
		}

		if (cmd.ID)
		{
			mUpdateCommands.insert(cmd.ID, cmd);
		}

		mSettings.endGroup();
	}

	foreach (auto cmd, mSettings.value("common/screenshot").toString().split(";"))
	{
		mScreenShotsCommands << cmd.toInt();
	}
}

//---------------------------------------------------------------------------
RemoteService::UpdateCommand RemoteService::findUpdateCommand(EUpdateType aType)
{
	foreach (auto command, mUpdateCommands.values())
	{
		if (command.type == aType)
		{
			return command;
		}
	}

	return RemoteService::UpdateCommand();
}

//---------------------------------------------------------------------------
void RemoteService::restartUpdateWatcher(QFileSystemWatcher * aWatcher)
{
	if (!aWatcher)
	{
		aWatcher = new QFileSystemWatcher(this);
		connect(aWatcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(onUpdateDirChanged()));
		connect(aWatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(onUpdateDirChanged()));
		aWatcher->addPath(mApplication->getWorkingDirectory() + "/update");
	}

	QStringList files;
	foreach (auto name, QDir(mApplication->getWorkingDirectory() + "/update", "*.rpt").entryInfoList(QDir::Files))
	{
		files << name.filePath();
	}

	if (!files.isEmpty())
	{
		aWatcher->addPaths(files);
	}
}

//---------------------------------------------------------------------------
void RemoteService::onModuleClosed(const QString & aModuleName)
{
	if (aModuleName == CWatchService::Modules::Updater &&
		checkUpdateReports() == 0)
	{
		// Апдейтер закрылся, но команда осталась в очереди - удаляем эту команду.
		foreach (auto cmd, mUpdateCommands.values())
		{
			if (cmd.isExternal() && cmd.status == Executing)
			{
				toLog(LogLevel::Warning, QString("Remove command %1 (type=%2) because updater module was closed. URL:%3.")
					.arg(cmd.ID).arg(cmd.type).arg(cmd.configUrl.toString()));

				updateCommandFinish(cmd.ID, Error);

				break;
			}
		}
	}
}

//---------------------------------------------------------------------------
void RemoteService::onDeviceStatusChanged(const QString & aConfigName)
{
	if (aConfigName.contains(SDK::Driver::CComponents::Watchdog) ||
		aConfigName.contains("Terminal", Qt::CaseInsensitive))
	{
		foreach(auto client, mMonitoringClients)
		{
			client->useCapability(PPSDK::IRemoteClient::ReportStatus);
		}
	}
}

//---------------------------------------------------------------------------
void RemoteService::timerEvent(QTimerEvent * aEvent)
{
	Q_UNUSED(aEvent)

	onCheckUpdateReports();

	startNextUpdateCommand();
}

//---------------------------------------------------------------------------
void RemoteService::updateCommandFinish(int aCmdID, EStatus aStatus, QVariantMap aParameters)
{
	emit commandStatusChanged(aCmdID, aStatus, aParameters);

	if (mUpdateCommands.contains(aCmdID))
	{
		QMutexLocker lock(&mCommandMutex);

		mUpdateCommands.remove(aCmdID);
		saveCommandQueue();
	}

	startNextUpdateCommand();
}

//---------------------------------------------------------------------------
void RemoteService::startNextUpdateCommand()
{
	UpdateCommand nextCommand;

	QMutexLocker lock(&mCommandMutex);

	foreach(int id, mUpdateCommands.keys())
	{
		if (mUpdateCommands[id].status == IRemoteService::Waiting)
		{
			nextCommand = mUpdateCommands[id];
			break;
		}
	}

	// запускаем следующую команду, если это возможно
	if (nextCommand.isValid())
	{
		mUpdateCommands.remove(nextCommand.ID);

		bool haveExecuted = false;

		foreach(auto cmd, mUpdateCommands.values())
		{
			if (cmd.status != IRemoteService::Waiting)
			{
				haveExecuted = true;
				break;
			}
		}

		if (!haveExecuted && mGenerateKeyCommand == 0 && mQueuedRebootCommands.isEmpty())
		{
			startUpdateCommand(nextCommand);
		}
		else
		{
			mUpdateCommands.insert(nextCommand.ID, nextCommand);
		}
	}
}

//---------------------------------------------------------------------------
RemoteService::UpdateCommand::UpdateCommand()
{
	ID = -1; 
	lastUpdate = QDateTime::currentDateTime();
}

//---------------------------------------------------------------------------
bool RemoteService::UpdateCommand::isValid() const
{
	return ID >= 0;
}

//---------------------------------------------------------------------------
bool RemoteService::UpdateCommand::isExternal() const
{
	return type != FirmwareUpload;
}

//---------------------------------------------------------------------------
