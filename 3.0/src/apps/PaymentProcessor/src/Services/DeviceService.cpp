/* @file Реализация сервиса для работы с устройствами. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QFileInfo>
#include <Common/QtHeadersEnd.h>

// STL
#include <algorithm>

// SDK
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/Drivers/InteractionTypes.h>
#include <SDK/Drivers/DeviceStatus.h>
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/HardwareConstants.h>
#include <SDK/Drivers/FR/FiscalFields.h>

// Modules
#include "DeviceManager/DeviceManager.h"

// Project
#include "DatabaseUtils/IHardwareDatabaseUtils.h"
#include "Services/ServiceNames.h"
#include "Services/PluginService.h"
#include "Services/DatabaseService.h"
#include "Services/SettingsService.h"
#include "System/IApplication.h"
#include "DeviceService.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
DeviceService * DeviceService::instance(IApplication * aApplication)
{
	return static_cast<DeviceService *>(aApplication->getCore()->getService(CServices::DeviceService));
}

//------------------------------------------------------------------------------
DeviceService::Status::Status() :
	mLevel(SDK::Driver::EWarningLevel::Error), mDescription(tr("#status_undefined")), mStatus(0)
{
}

//------------------------------------------------------------------------------
DeviceService::Status::Status(SDK::Driver::EWarningLevel::Enum aLevel, const QString & aDescription, int aStatus) :
	mLevel(aLevel), mDescription(aDescription), mStatus(aStatus)
{
}

//------------------------------------------------------------------------------
DeviceService::Status::Status(const Status & aStatus) :
	mLevel(aStatus.mLevel), mDescription(aStatus.mDescription), mStatus(aStatus.mStatus)
{
}

//------------------------------------------------------------------------------
SDK::Driver::EWarningLevel::Enum DeviceService::Status::level() const
{
	return mLevel;
}

//------------------------------------------------------------------------------
const QString & DeviceService::Status::description() const
{
	return mDescription;
}

//------------------------------------------------------------------------------
bool DeviceService::Status::isMatched(SDK::Driver::EWarningLevel::Enum aLevel) const
{
	return (mLevel <= aLevel && DSDK::getStatusType(mStatus) != DSDK::EStatus::Interface);
}

//------------------------------------------------------------------------------
DeviceService::DeviceService(IApplication * aApplication)
	: mDeviceManager(nullptr),
	mAccessMutex(QMutex::Recursive),
	mApplication(aApplication),
	mLog(aApplication->getLog()),
	mDatabaseUtils(nullptr)
{
	connect(&mDetectionResult, SIGNAL(finished()), this, SLOT(onDetectionFinished()));

	mDeviceCreationOrder[DSDK::CComponents::Watchdog] = EDeviceCreationOrder::AtStart;
	mDeviceCreationOrder[DSDK::CComponents::Health]   = EDeviceCreationOrder::AtStart;

#ifdef TC_USE_TOKEN
	mDeviceCreationOrder[DSDK::CComponents::Token] = EDeviceCreationOrder::AtStart;
#endif
}

//------------------------------------------------------------------------------
bool DeviceService::initialize()
{
	// Вытаскиваем загрузчик плагинов.
	PluginService * pluginManager = PluginService::instance(mApplication);

	mDeviceManager = new DeviceManager(pluginManager->getPluginLoader());
	mDeviceManager->setLog(mApplication->getLog());

	// Здесь используем DirectConnction для того, чтобы инициализация устройства при автопоиске
	// производилась из потока автопоиска и не грузила основной поток.
	connect(mDeviceManager, SIGNAL(deviceDetected(const QString &, SDK::Driver::IDevice *)), this,
		SLOT(onDeviceDetected(const QString &, SDK::Driver::IDevice *)), Qt::DirectConnection);

	mDatabaseUtils = DatabaseService::instance(mApplication)->getDatabaseUtils<IHardwareDatabaseUtils>();

	mDeviceManager->initialize();

	mIntegratedDrivers.initialize(mDeviceManager);

	return true;
}

//------------------------------------------------------------------------------
void DeviceService::finishInitialize()
{
	// Повторный эмит статусов всех устройств
	QMapIterator<QString, Status> it(mDeviceStatusCache);

	while (it.hasNext()) 
	{
		it.next();
		Status status = it.value();

		emit deviceStatusChanged(it.key(), status.mLevel, status.mDescription, status.mStatus);
	}

	// Создаём устройства, которым положено запускаться при старте системы
	foreach (auto deviceInstance, getConfigurations(false))
	{
		QString deviceType = deviceInstance.section('.', 2, 2);

		if (mDeviceCreationOrder.contains(deviceType) && mDeviceCreationOrder.value(deviceType) == EDeviceCreationOrder::AtStart)
		{
			acquireDevice(deviceInstance);
		}
	}

	LOG(mLog, LogLevel::Normal, "DeviceService initialized.");
}

//---------------------------------------------------------------------------
bool DeviceService::canShutdown()
{
	return true;
}

//------------------------------------------------------------------------------
bool DeviceService::shutdown()
{
	if (mDetectionResult.isRunning())
	{
		stopDetection();

		mDetectionResult.waitForFinished();
	}

	releaseAll();

	delete mDeviceManager;

	return true;
}

//------------------------------------------------------------------------------
QString DeviceService::getName() const
{
	return CServices::DeviceService;
}

//------------------------------------------------------------------------------
QVariantMap DeviceService::getParameters() const
{
	return QVariantMap();
}

//------------------------------------------------------------------------------
void DeviceService::resetParameters(const QSet<QString> &)
{
}

//------------------------------------------------------------------------------
const QSet<QString> & DeviceService::getRequiredServices() const
{
	static QSet<QString> requiredServices = QSet<QString>()
		<< CServices::DatabaseService
		<< CServices::PluginService;

	return requiredServices;
}

//------------------------------------------------------------------------------
DeviceService::~DeviceService()
{
}

//------------------------------------------------------------------------------
void DeviceService::detect(const QString & aFilter)
{
	mAccessMutex.lock();

	mDetectionResult.setFuture(QtConcurrent::run(this, &DeviceService::doDetect, aFilter));
}

//------------------------------------------------------------------------------
void DeviceService::doDetect(const QString & aFilter)
{
	LOG(mLog, LogLevel::Normal, "Starting device detection...");
	mDeviceManager->detect(aFilter);
}

//------------------------------------------------------------------------------
void DeviceService::stopDetection()
{
	mDeviceManager->stopDetection();
}

//------------------------------------------------------------------------------
void DeviceService::onDetectionFinished()
{
	mAccessMutex.unlock();

	emit detectionStopped();
	LOG(mLog, LogLevel::Normal, "Device detection is completed or terminated.");
}

//------------------------------------------------------------------------------
void DeviceService::onDeviceDetected(const QString & aConfigName, DSDK::IDevice * aDevice)
{
	mAcquiredDevices.insert(aConfigName, aDevice);

	initializeDevice(aConfigName, aDevice);

	emit deviceDetected(aConfigName);
}

//------------------------------------------------------------------------------
bool DeviceService::initializeDevice(const QString & aConfigName, DSDK::IDevice * aDevice)
{
	// Устанавливаем параметры, необходимые для инициализации устройства.
	foreach (const QString & deviceType, mInitParameters.keys())
	{
		if (aConfigName.indexOf(deviceType) != -1)
		{
			aDevice->setDeviceConfiguration(mInitParameters[deviceType]);
		}
	}

	// Подписываемся на события об изменении статуса устройства.
	aDevice->subscribe(SDK::Driver::IDevice::StatusSignal, this, SLOT(onDeviceStatus(SDK::Driver::EWarningLevel::Enum, const QString &, int)));

	// Инициализируем устройство.
	aDevice->initialize();

	// Добавляем имя устройства как параметр в БД.
	mDatabaseUtils->setDeviceParam(aConfigName, PPSDK::CDatabaseConstants::Parameters::DeviceName, aDevice->getName());

	return true;
}

//------------------------------------------------------------------------------
DSDK::IDevice * DeviceService::acquireDevice(const QString & aInstancePath)
{
	QMutexLocker lock(&mAccessMutex);

	QString instancePath(aInstancePath);
	QString configInstancePath(instancePath);
	mDeviceManager->checkITInstancePath(instancePath);

	if (mAcquiredDevices.contains(instancePath))
	{
		return mAcquiredDevices.value(instancePath);
	}
	else
	{
		DSDK::IDevice * device = mDeviceManager->acquireDevice(instancePath, configInstancePath);

		if (device)
		{
			mAcquiredDevices.insert(instancePath, device);

			QMap<int, PPSDK::SKeySettings> keys = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>()->getKeys();

			if (keys.contains(0))
			{
				QVariantMap configuration;
				configuration.insert(CFiscalSDK::AutomaticNumber, keys.value(0).ap);
				device->setDeviceConfiguration(configuration);
			}

			initializeDevice(instancePath, device);
		}

		return device;
	}
}

//------------------------------------------------------------------------------
void DeviceService::releaseDevice(DSDK::IDevice * aDevice)
{
	QMutexLocker lock(&mAccessMutex);

	mAcquiredDevices.remove(mAcquiredDevices.key(aDevice));

	mDeviceManager->releaseDevice(aDevice);
}

//------------------------------------------------------------------------------
PPSDK::IDeviceService::UpdateFirmwareResult DeviceService::updateFirmware(const QByteArray & aFirmware, const QString & aDeviceGUID)
{
	//TODO: сделать проверку выше (например, в апдейтере) на то, что файл открывается и что он не пуст
	QMutexLocker lock(&mAccessMutex);

	DSDK::IDevice * device = nullptr;

	LOG(mLog, LogLevel::Normal, QString("Start update firmware device with GUID %1.").arg(aDeviceGUID));

	for (TAcquiredDevices::iterator it = mAcquiredDevices.begin(); it != mAcquiredDevices.end(); ++it)
	{
		if (it.key().contains(aDeviceGUID))
		{
			device = it.value();
		}
	}

	if (!device)
	{
		LOG(mLog, LogLevel::Error, QString("No device with GUID %1 for updating the firmware.").arg(aDeviceGUID));
		return IDeviceService::NoDevice;
	}

	if (!device->canUpdateFirmware())
	{
		LOG(mLog, LogLevel::Error, "The device cannot be updated.");
		return IDeviceService::CantUpdate;
	}

	device->updateFirmware(aFirmware);

	return IDeviceService::OK;
}

//------------------------------------------------------------------------------
QString DeviceService::createDevice(const QString & aDriverPath, const QVariantMap & aConfig)
{
	QMutexLocker lock(&mAccessMutex);

	QString driverPath(aDriverPath);
	mIntegratedDrivers.checkDriverPath(driverPath, aConfig);

	TNamedDevice result = mDeviceManager->createDevice(driverPath, aConfig);

	if (result.second)
	{
		mAcquiredDevices.insert(result.first, result.second);

		QMap<int, PPSDK::SKeySettings> keys = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>()->getKeys();

		if (keys.contains(0))
		{
			QVariantMap configuration;
			configuration.insert(CFiscalSDK::AutomaticNumber, keys.value(0).ap);
			result.second->setDeviceConfiguration(configuration);
		}

		initializeDevice(result.first, result.second);
	}
	else
	{
		LOG(mLog, LogLevel::Error, QString("Failed to create device %1 .").arg(driverPath));
	}

	return result.first;
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::TModelList DeviceService::getModelList(const QString & aFilter) const
{
	PPSDK::TModelList result = mDeviceManager->getModelList(aFilter);
	mIntegratedDrivers.filterModelList(result);

	return result;
}

//------------------------------------------------------------------------------
QStringList DeviceService::getAcquiredDevicesList() const
{
	QMutexLocker lock(&mAccessMutex);

	return mAcquiredDevices.keys();
}

//------------------------------------------------------------------------------
void DeviceService::releaseAll()
{
	QMutexLocker lock(&mAccessMutex);

	foreach (DSDK::IDevice * device, mAcquiredDevices.values())
	{
		releaseDevice(device);
	}
}

//------------------------------------------------------------------------------
QString DeviceService::getDeviceConfigName(DSDK::IDevice * aDevice)
{
	QMutexLocker lock(&mAccessMutex);

	return mAcquiredDevices.key(aDevice);
}

//------------------------------------------------------------------------------
void DeviceService::setInitParameters(const QString & aDeviceType, const QVariantMap & aParameters)
{
	mInitParameters[aDeviceType] = aParameters;
}

//------------------------------------------------------------------------------
QStringList DeviceService::getConfigurations(bool aAllowOldConfigs) const
{
	PPSDK::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();
	QStringList configurations = settings->getDeviceList();

	if (aAllowOldConfigs)
	{
		for (int i = 0; i < configurations.size(); ++i)
		{
			mDeviceManager->checkITInstancePath(configurations[i]);
		}
	}

	// Обязательные устройства "присуствуют" всегда
	QStringList driverList = mDeviceManager->getDriverList();
	QStringList requiredDevices; 
	
	requiredDevices << DSDK::CComponents::Health;
#ifdef TC_USE_TOKEN
	requiredDevices << DSDK::CComponents::Token;
#endif

	foreach(QString type, requiredDevices)
	{
		auto isDevicePresent = [type](const QString & aConfigurationName) -> bool { return aConfigurationName.contains(type); };

		if (std::find_if(configurations.begin(), configurations.end(), isDevicePresent) == configurations.end() &&
			std::find_if(driverList.begin(), driverList.end(), isDevicePresent) != driverList.end())
		{
			configurations << *std::find_if(driverList.begin(), driverList.end(), isDevicePresent);
		}
	}

	return configurations;
}

//------------------------------------------------------------------------------
bool DeviceService::saveConfigurations(const QStringList & aConfigList)
{
	// Проходимся по всем задействованным устройствам и сохраняем конфигурацию.
	foreach (const QString & configName, aConfigList)
	{
		DSDK::IDevice * device = acquireDevice(configName);

		if (device)
		{
			mDeviceManager->saveConfiguration(device);
		}
		else
		{
			LOG(mLog, LogLevel::Error, QString("Failed to set device configuration. No such device: %1.").arg(configName));
		}
	}

	PPSDK::TerminalSettings * settings = SettingsService::instance(mApplication)->getAdapter<PPSDK::TerminalSettings>();
	settings->setDeviceList(aConfigList);

	mDatabaseUtils->removeUnknownDevice(aConfigList);

	// Посылаем сигнал, чтобы остальные сервисы обновили устройства.
	emit configurationUpdated();

	return true;
}

//------------------------------------------------------------------------------
QVariantMap DeviceService::getDeviceConfiguration(const QString & aConfigName)
{
	QMutexLocker lock(&mAccessMutex);

	DSDK::IDevice * device = acquireDevice(aConfigName);

	if (device)
	{
		return mDeviceManager->getDeviceConfiguration(device);
	}
	else
	{
		LOG(mLog, LogLevel::Error, QString("Failed to set device configuration. No such device: %1.").arg(aConfigName));
		return QVariantMap();
	}
}

//------------------------------------------------------------------------------
void DeviceService::setDeviceConfiguration(const QString & aConfigName, const QVariantMap & aConfig)
{
	DSDK::IDevice * device = acquireDevice(aConfigName);

	// Производим переинициализацию устройства.
	if (device)
	{
		device->release();

		{
			QMutexLocker lock(&mAccessMutex);

			mDeviceManager->setDeviceConfiguration(device, aConfig);
		}

		device->initialize();
	}
	else
	{
		LOG(mLog, LogLevel::Error, QString("Failed to set device configuration. No such device: %1.").arg(aConfigName));
	}
}

//------------------------------------------------------------------------------
SDK::Plugin::TParameterList DeviceService::getDriverParameters(const QString & aDriverPath) const
{
	SDK::Plugin::TParameterList result = mDeviceManager->getDriverParameters(aDriverPath);
	mIntegratedDrivers.filterDriverParameters(aDriverPath, result);

	return result;
}

//------------------------------------------------------------------------------
QStringList DeviceService::getDriverList() const
{
	// Выдаем список драйверов всех устройств, кроме портов.
	QStringList result = mDeviceManager->getDriverList().filter(QRegExp(".+\\.Driver\\.(?!IOPort)"));
	mIntegratedDrivers.filterDriverList(result);

	return result;
}

//------------------------------------------------------------------------------
void DeviceService::overwriteDeviceStatus(DSDK::IDevice * aDevice, DSDK::EWarningLevel::Enum aLevel, const QString & aDescription, int aStatus)
{
	// Берём последний статус и пишем в БД, только если новый статус отличается по WarningLevel
	QString configName = mAcquiredDevices.key(aDevice);

	if (!mDeviceStatusCache.contains(configName) ||
		mDeviceStatusCache[configName].mLevel != aLevel ||
		DSDK::getStatusType(mDeviceStatusCache[configName].mStatus) == DSDK::EStatus::Interface)
	{
		Status status(aLevel, aDescription, aStatus);

		statusChanged(aDevice, status);
	}
}

//------------------------------------------------------------------------------
void DeviceService::onDeviceStatus(DSDK::EWarningLevel::Enum aLevel, const QString & aDescription, int aStatus)
{
	LogLevel::Enum logLevel = (aLevel == DSDK::EWarningLevel::OK) ? LogLevel::Normal : (aLevel == DSDK::EWarningLevel::Error) ? LogLevel::Error : LogLevel::Warning;
	LOG(mLog, logLevel, QString("Received statuses: %1, status %2").arg(aDescription).arg(aStatus));

	DSDK::IDevice * device = dynamic_cast<DSDK::IDevice *>(sender());

	if (device)
	{
		Status status(aLevel, aDescription, aStatus);

		statusChanged(device, status);

		// Удаляем неиспользуемые устройства.
		foreach (auto configName, mDeviceStatusCache.keys())
		{
			if (!mAcquiredDevices.contains(configName))
			{
				mDeviceStatusCache.remove(configName);
			}
		}
	}
}

//------------------------------------------------------------------------------
void DeviceService::statusChanged(DSDK::IDevice * aDevice, Status & aStatus)
{
	DSDK::EStatus::Enum statusType = DSDK::getStatusType(aStatus.mStatus);

	// Если данный статус зарегистрирован в исключениях, то мы его не обрабатываем.
	if (statusType == DSDK::EStatus::Service)
	{
		return;
	}

	// Запоминаем последние статусы.
	QString configName = mAcquiredDevices.key(aDevice);
	mDeviceStatusCache[configName] = aStatus;

	LogLevel::Enum logLevel = (aStatus.mLevel == DSDK::EWarningLevel::OK) ? LogLevel::Normal : (aStatus.mLevel == DSDK::EWarningLevel::Error) ? LogLevel::Error : LogLevel::Warning;
	LOG(mLog, logLevel, QString("Send statuses: %1, status %2, device %3").arg(aStatus.mDescription).arg(aStatus.mStatus).arg(configName));

	if (statusType != DSDK::EStatus::Interface)
	{
		// Обновляем имя и информацию об устройстве в базе, т.к. возможно уточнено имя после обращения к устройству.
		mDatabaseUtils->setDeviceParam(configName, PPSDK::CDatabaseConstants::Parameters::DeviceName, aDevice->getName());
		QVariantMap deviceConfig = aDevice->getDeviceConfiguration();
	
		if (deviceConfig.contains(CHardwareSDK::DeviceData))
		{
			mDatabaseUtils->setDeviceParam(configName, PPSDK::CDatabaseConstants::Parameters::DeviceInfo, deviceConfig[CHardwareSDK::DeviceData].toString());
		}

		// Пишем статус в БД.
		mDatabaseUtils->addDeviceStatus(configName, aStatus.mLevel, aStatus.mDescription);
	}

	emit deviceStatusChanged(configName, aStatus.mLevel, aStatus.mDescription, aStatus.mStatus);
}

//------------------------------------------------------------------------------
QSharedPointer<PPSDK::IDeviceStatus> DeviceService::getDeviceStatus(const QString & aConfigName)
{
	if (mDeviceStatusCache.contains(aConfigName))
	{
		return QSharedPointer<PPSDK::IDeviceStatus>(new Status(mDeviceStatusCache.value(aConfigName)));
	}

	return QSharedPointer<PPSDK::IDeviceStatus>(nullptr);
}

//------------------------------------------------------------------------------
