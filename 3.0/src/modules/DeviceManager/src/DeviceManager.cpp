/* @file Менеджер устройств. */

// STL
#include <algorithm>
#include <functional>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QFutureSynchronizer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/InteractionTypes.h>
#include <SDK/Drivers/HardwareConstants.h>
#include <SDK/Drivers/DetectingPriority.h>

// Project
#include "RenamingPluginPaths.h"
#include "DeviceManager.h"

using namespace SDK::Driver;
using namespace SDK::Plugin;

//--------------------------------------------------------------------------------
DeviceManager::DeviceManager(IPluginLoader * aPluginLoader) : mPluginLoader(aPluginLoader), mStopFlag(1)
{
	qRegisterMetaType<TNamedDevice>("TNamedDevice");
}

//--------------------------------------------------------------------------------
DeviceManager::~DeviceManager()
{
}

//--------------------------------------------------------------------------------
bool DeviceManager::initialize()
{
	QString pluginPath;

	QStringList driverPlugins = mPluginLoader->getPluginList(QRegExp("\\.Driver\\."));

	// Составляем таблицу необходимых устройств.
	foreach (pluginPath, driverPlugins)
	{
		TParameterList parameters = mPluginLoader->getPluginParametersDescription(pluginPath);
		mDriverParameters.insert(pluginPath, parameters);

		// Получаем необходимый ресурс.
		SPluginParameter requiredResource = findParameter(CHardwareSDK::RequiredResource, parameters);

		QString required;

		if (requiredResource.isValid())
		{
			required = requiredResource.defaultValue.toString();
		}

		mRequiredResources.insert(pluginPath, required);

		// Получаем список системных имен, если он содержится в драйвере.
		SPluginParameter nameParameter = findParameter(CHardwareSDK::SystemName, parameters);

		if (!nameParameter.name.isEmpty())
		{
			QStringList systemNames = nameParameter.possibleValues.keys();

			if (!systemNames.isEmpty())
			{
				foreach (auto systemName, systemNames)
				{
					mRDSystemNames.insert(pluginPath, systemName);
					mFreeSystemNames.insert(systemName);
				}
			}
			else
			{
				mRDSystemNames.insert(pluginPath, "");
			}
		}

		// Получаем список моделей.
		SPluginParameter modelList = findParameter(CHardwareSDK::ModelName, parameters);
		mDriverList[pluginPath].append(modelList.possibleValues.keys());
	}

	// Устанавливаем максимальное количество потоков в пуле.
	QThreadPool::globalInstance()->setMaxThreadCount(32);

	return true;
}

//--------------------------------------------------------------------------------
void DeviceManager::shutdown()
{
	// Перенесено в DeviceService.
}

//--------------------------------------------------------------------------------
IDevice * DeviceManager::acquireDevice(const QString & aInstancePath, const QString & aConfigInstancePath)
{
	QString configInstancePath = aConfigInstancePath.isEmpty() ? aInstancePath : aConfigInstancePath;

	// Создаем нужный плагин.
	IPlugin * plugin = mPluginLoader->createPlugin(aInstancePath, configInstancePath);
	IDevice * device = dynamic_cast<IDevice *>(plugin);

	if (!device)
	{
		toLog(LogLevel::Error, QString("Failed to create device '%1'.").arg(aInstancePath));

		if (plugin)
		{
			mPluginLoader->destroyPlugin(plugin);
		}

		return nullptr;
	}

	// Получаем сохраненную конфигу
	QVariantMap config = device->getDeviceConfiguration();
	QString driverPath = aInstancePath.section(CPlugin::InstancePathSeparator, 0, 0);

	// Устройство может быть привязано к системному имени (например COM-порты).
	if (mRDSystemNames.contains(driverPath))
	{
		// Удаляем системное имя из списка доступных.
		QString systemName = config[CHardwareSDK::SystemName].toString();

		if (!mFreeSystemNames.contains(systemName))
		{
			// Указанное системное имя недоступно.
			toLog(LogLevel::Warning, QString("Required system name %1 is not available.").arg(systemName));

			TParameterList & parameters = mDriverParameters[driverPath];
			auto it = std::find_if(parameters.begin(), parameters.end(), [&] (const SPluginParameter & aParameter) -> bool { return aParameter.name == CHardwareSDK::SystemName; });

			if (it != parameters.end())
			{
				it->possibleValues.insert(systemName, systemName);
			}

			mRDSystemNames.insert(driverPath, systemName);
		}

		mFreeSystemNames.remove(systemName);
	}

	// Проверяем, есть ли ссылка не необходимый ресурс.
	if (findParameter(CHardwareSDK::RequiredResource, mDriverParameters.value(driverPath)).isValid())
	{
		QString resourceName = config[CHardwareSDK::RequiredResource].toString();
		QString configResourceName = resourceName;
		checkITInstancePath(resourceName);
		IDevice * requiredDevice = acquireDevice(resourceName, configResourceName);

		if (!requiredDevice)
		{
			toLog(LogLevel::Error, QString("Failed to create required resource %1 for device %2.").arg(config[CHardwareSDK::RequiredResource].toString()).arg(aInstancePath));
			mPluginLoader->destroyPlugin(plugin);

			return nullptr; 
		}

		config[CHardwareSDK::RequiredDevice] = QVariant::fromValue(requiredDevice);
		device->setDeviceConfiguration(config);

		mDeviceDependencyMap.insert(device, requiredDevice);
	}

	QString itType = driverPath.split(".").size() >= 4 ? driverPath.split(".")[3] : "";

	// Если устройство не системное - назначаем ему log и ставим тип поиска устройсва.
	if (!mRDSystemNames.contains(driverPath) ||
		itType == CInteractionTypes::External)
	{
		setDeviceLog(device, false);

		QVariantMap localConfig;
		localConfig[CHardwareSDK::SearchingType] = CHardwareSDK::SearchingTypes::Loading;
		device->setDeviceConfiguration(localConfig);
	}

	mAcquiredDevices << driverPath;

	device->subscribe(IDevice::ConfigurationChangedSignal, this, SLOT(onConfigurationChanged()));

	return device;
}

//--------------------------------------------------------------------------------
void DeviceManager::onConfigurationChanged()
{
	IDevice * device = dynamic_cast<IDevice *>(sender());

	dynamic_cast<IPlugin *>(device)->saveConfiguration();
}

//--------------------------------------------------------------------------------
void DeviceManager::releaseDevice(IDevice * aDevice)
{
	// Освобождаем устройство и все с ним связанные ресурсы.
	QVariantMap config = aDevice->getDeviceConfiguration();

	aDevice->release();

	// Уничтожаем инстанс плагина.
	IPlugin * plugin = dynamic_cast<IPlugin *>(aDevice);
	
	// #55103 - не сохраняем новую конфигурацию устройства, при выходе из клиента,
	//          т.к. иначе не сможем откатитсья на старую версию ТК.
	// plugin->saveConfiguration();

	QString driverPath = plugin->getConfigurationName().section(CPlugin::InstancePathSeparator, 0, 0);
	mPluginLoader->destroyPlugin(plugin);

	if (mRDSystemNames.contains(driverPath))
	{
		QString systemName = config[CHardwareSDK::SystemName].toString();
		mFreeSystemNames.insert(systemName);
	}

	mDevicesLogData.remove(aDevice);

	foreach (auto device, mDeviceDependencyMap.values(aDevice))
	{
		// Освобождаем зависимый ресурс.
		releaseDevice(device);

		mDeviceDependencyMap.remove(aDevice, device);
	}
}

//--------------------------------------------------------------------------------
// TODO: обобщить через acquireDevice.
TNamedDevice DeviceManager::createDevice(const QString & aDriverPath, const QVariantMap & aConfig, bool aDetecting)
{
	// Создаем нужный плагин.
	IPlugin * plugin = mPluginLoader->createPlugin(aDriverPath);
	IDevice * device = dynamic_cast<IDevice *>(plugin);

	if (!device)
	{
		toLog(LogLevel::Error, QString("Failed to create device '%1'.").arg(aDriverPath));

		if (plugin)
		{
			mPluginLoader->destroyPlugin(plugin);
		}

		return TNamedDevice("", nullptr);
	}

	QString configPath = plugin->getConfigurationName();
	auto config = aConfig;

	// Устройство может быть привязано к системному имени (например COM-порты).
	if (mRDSystemNames.contains(aDriverPath))
	{
		// Удаляем системное имя из списка доступных.
		QString systemName = config[CHardwareSDK::SystemName].toString();

		if (!systemName.isEmpty() && !mFreeSystemNames.contains(systemName))
		{
			// Указанное системное имя недоступно.
			mPluginLoader->destroyPlugin(plugin);
			toLog(LogLevel::Error, QString("Required system name %1 is not available.").arg(systemName));

			return TNamedDevice("", nullptr);
		}

		mFreeSystemNames.remove(systemName);
	}

	// Создаем необходимый ресурс.
	QString requiredResourcePath = mRequiredResources.contains(aDriverPath) ? mRequiredResources[aDriverPath] : "";

	if (!requiredResourcePath.isEmpty())
	{
		TNamedDevice requiredDevice = createDevice(requiredResourcePath, config, aDetecting);

		if (!requiredDevice.second)
		{
			mPluginLoader->destroyPlugin(plugin);
			toLog(LogLevel::Error, QString("Failed to create required resource '%1'.").arg(requiredResourcePath));

			return TNamedDevice("", nullptr);
		}

		config[CHardwareSDK::RequiredResource] = requiredDevice.first;
		config[CHardwareSDK::RequiredDevice] = QVariant::fromValue(requiredDevice.second);
		mDeviceDependencyMap.insert(device, requiredDevice.second);
	}

	device->setDeviceConfiguration(config);

	if (!aDetecting)
	{
		setDeviceLog(device, aDetecting);
	}

	device->subscribe(IDevice::ConfigurationChangedSignal, this, SLOT(onConfigurationChanged()));

	return TNamedDevice(configPath, device);
}

//--------------------------------------------------------------------------------
QMap<QString, QStringList> DeviceManager::getModelList(const QString & /*aFilter*/)
{
	return mDriverList;
}

//--------------------------------------------------------------------------------
TNamedDevice DeviceManager::findDevice(IDevice * aRequired, const QStringList & aDevicesToFind)
{
	QMap<QString, IDevice *> targetDevices;
	QMap<QString, IDevice::IDetectingIterator *> detectingIterators;
	QStringList configNames;

	IDevice * result = nullptr;

	QString logFileName = getDeviceLogName(aRequired, true);
	ILog * log = ILog::getInstance(logFileName);
	aRequired->setLog(log);

	// Создаем все устройства, которые собираемся искать.
	foreach (const QString & targetDevice, aDevicesToFind)
	{
		// Создаем нужный плагин.
		IPlugin * plugin = mPluginLoader->createPlugin(targetDevice);
		IDevice * device = dynamic_cast<IDevice *>(plugin);

		if (!device)
		{
			toLog(LogLevel::Error, QString("Failed to create device %1 .").arg(targetDevice));

			if (plugin)
			{
				mPluginLoader->destroyPlugin(plugin);
			}

			continue;
		}

		device->setLog(log);

		QVariantMap config;

		// Сохраняем в конфигурации параметры по умолчанию
		TParameterList parameters = mDriverParameters[targetDevice];

		for (auto it = parameters.begin(); it != parameters.end(); ++it)
		{
			config.insert(it->name, it->defaultValue);
		}

		config[CHardwareSDK::SearchingType] = CHardwareSDK::SearchingTypes::AutoDetecting;
		config[CHardwareSDK::RequiredDevice] = QVariant::fromValue(aRequired);
		config[CHardwareSDK::RequiredResource] = dynamic_cast<IPlugin *>(aRequired)->getConfigurationName();

		device->setDeviceConfiguration(config);
		IDevice::IDetectingIterator * detectingIterator = device->getDetectingIterator();

		if (detectingIterator)
		{
			targetDevices.insert(plugin->getConfigurationName(), device);
			detectingIterators.insert(plugin->getConfigurationName(), detectingIterator);
			configNames.append(plugin->getConfigurationName());
		}
	}

	QStringList nonMarketDetectedDriverNames;

	// Делаем одну итерацию поиска для каждого устройства.
	while (!configNames.isEmpty())
	{
		foreach (const QString & targetDevice, configNames)
		{
			// Если возможности для перебора исчерпаны, то удаляем итератор.
			IDevice::IDetectingIterator * detectIterator = detectingIterators.value(targetDevice);
			IDevice * device = targetDevices[targetDevice];

			if (mStopFlag)
			{
				configNames.clear();
				break;
			}

			if (!detectIterator->moveNext() || isDetected(targetDevice))
			{
				configNames.removeOne(targetDevice);
				continue;
			}

			if (detectIterator->find())
			{
				// Устройство обнаружено.
				{
					QMutexLocker lock(&mAccessMutex);

					mDeviceDependencyMap.insert(device, aRequired);
				}

				// Устанавливаем новый лог.
				setDeviceLog(device, false);

				emit deviceDetected(targetDevice, device);

				if (device->getDeviceConfiguration()[CHardwareSDK::Existence].toString() != CHardwareSDK::ExistenceTypes::Multiple)
				{
					markDetected(targetDevice);
				}
				else
				{
					nonMarketDetectedDriverNames << targetDevice;
				}

				result = device;
				configNames.clear();

				break;
			}
		}
	}

	foreach (const QString & driverName, nonMarketDetectedDriverNames)
	{
		markDetected(driverName);
	}

	// Уничтожаем все драйверы, устройства которых не были обнаружены.
	foreach (IDevice * device, targetDevices.values())
	{
		if (device != result)
		{
			mPluginLoader->destroyPlugin(dynamic_cast<IPlugin *>(device));
		}
	}

	return TNamedDevice("", result);
}

//--------------------------------------------------------------------------------
void DeviceManager::findSimpleDevice(const QString & aDriverName, QStringList & aResult, TFallbackDevices & aFallbackDevices, QStringList & aNonMarketDetectedDriverNames)
{
	bool detected = false;

	TNamedDevice namedDevice;
	QMetaObject::invokeMethod(this, "createDevice", Qt::BlockingQueuedConnection, QReturnArgument<TNamedDevice>("TNamedDevice", namedDevice),
		Q_ARG(QString, aDriverName), Q_ARG(QVariantMap, QVariantMap()), Q_ARG(bool, true));

	if (!namedDevice.second)
	{
		return;
	}

	QVariantMap config;
	config[CHardwareSDK::SearchingType] = CHardwareSDK::SearchingTypes::AutoDetecting;
	namedDevice.second->setDeviceConfiguration(config);

	IDevice::IDetectingIterator * detectingIterator = namedDevice.second->getDetectingIterator();
	int iterations = 0;

	while (detectingIterator && detectingIterator->moveNext())
	{
		iterations++;
		setDeviceLog(namedDevice.second, true);

		if (!isDetected(aDriverName) && detectingIterator->find())
		{
			detected = true;

			SPluginParameter parameter = findParameter(CHardwareSDK::DetectingPriority, mDriverParameters.value(aDriverName));

			if (!parameter.isValid() || parameter.defaultValue.toInt() != DSDK::EDetectingPriority::Fallback)
			{
				aResult.append(namedDevice.first);

				if (namedDevice.second->getDeviceConfiguration()[CHardwareSDK::Existence].toString() != CHardwareSDK::ExistenceTypes::Multiple)
				{
					markDetected(aDriverName);
				}
				else
				{
					aNonMarketDetectedDriverNames << aDriverName;
				}

				setDeviceLog(namedDevice.second, false);

				emit deviceDetected(namedDevice.first, namedDevice.second);
			}
			else
			{
				aFallbackDevices.append(namedDevice);
			}

			if (namedDevice.second->getDeviceConfiguration()[CHardwareSDK::Existence].toString() != CHardwareSDK::ExistenceTypes::Multiple)
			{
				break;
			}

			QMetaObject::invokeMethod(this, "createDevice", Qt::BlockingQueuedConnection, QReturnArgument<TNamedDevice>("TNamedDevice", namedDevice),
				Q_ARG(QString, aDriverName), Q_ARG(QVariantMap, QVariantMap()), Q_ARG(bool, true));

			detected = false;

			if (!namedDevice.second)
			{
				break;
			}

			namedDevice.second->setDeviceConfiguration(config);
			detectingIterator = namedDevice.second->getDetectingIterator();
			int maxIterations = iterations;

			while (maxIterations--)
			{
				detectingIterator->moveNext();
			}
		}
	}

	if (!detected)
	{
		releaseDevice(namedDevice.second);
	}
}

//--------------------------------------------------------------------------------
void DeviceManager::logRequiedDeviceData()
{
	QStringList interactionTypes = QStringList()
		<< CInteractionTypes::COM
		<< CInteractionTypes::USB
		<< CInteractionTypes::LibUSB;

	QStringList driverList = mDriverList.keys();

	foreach (const QString & interactionType, interactionTypes)
	{
		foreach (const QString & driverName, driverList)
		{
			if (driverName.split('.').last() == interactionType)
			{
				IDevice * required = createDevice(driverName, QVariantMap(), true).second;

				if (required)
				{
					QString logName = QString("autodetect/%1 ports").arg(interactionType);
					ILog * log = ILog::getInstance(logName);
					required->setLog(log);
					required->initialize();
					releaseDevice(required);
				};
			}
		}
	}
}

//--------------------------------------------------------------------------------
QStringList DeviceManager::detect(const QString & /*aDeviceType*/)
{
	logRequiedDeviceData();

	QStringList result;

	mStopFlag = 0;
	mDetectedDeviceTypes.clear();
	QList<TNamedDevice> fallbackDevices;
	QStringList nonMarketDetectedDriverNames;

	// Создаем устройства, для работы которых ничего не требуется. Или почти ничего.
	typedef QSet<QString> QStringSet;
	QStringList allDevices = mRequiredResources.keys();
	QStringList TCPDevices = allDevices.filter(CInteractionTypes::TCP, Qt::CaseInsensitive);
	QStringList simpleDevices = mRequiredResources.keys("") + TCPDevices;

	QStringSet OPOSDevices = simpleDevices.filter(CInteractionTypes::OPOS, Qt::CaseInsensitive).toSet();
	QStringSet nonOPOSDevices = simpleDevices.toSet() - OPOSDevices;

	foreach (const QString & driverName, nonOPOSDevices)
	{
		if (!mRDSystemNames.contains(driverName))
		{
			findSimpleDevice(driverName, result, fallbackDevices, nonMarketDetectedDriverNames);
		}
	}

	foreach (const QString & driverName, OPOSDevices)
	{
		if (!mRDSystemNames.contains(driverName))
		{
			findSimpleDevice(driverName, result, fallbackDevices, nonMarketDetectedDriverNames);
		}
	}

	foreach (const QString & driverName, nonMarketDetectedDriverNames)
	{
		markDetected(driverName);
	}

	QFutureSynchronizer<TNamedDevice> synchronizer;
	QVector<IDevice *> requiredList;
	
	// Для каждого ресурса берем список системных имен, связанных с ресурсом.
	foreach (const QString & requiredDevice, mRDSystemNames.keys().toSet())
	{
		QStringList devicesToFind = mRequiredResources.keys(requiredDevice);

		if (devicesToFind.isEmpty() || requiredDevice.contains(CInteractionTypes::TCP))
		{ 
			continue;
		}

		qSort(devicesToFind.begin(), devicesToFind.end(), std::bind(&DeviceManager::deviceSortPredicate, this, std::placeholders::_1, std::placeholders::_2));

		// Для каждого системного имени создаем устройство.
		foreach (const QString & systemName, mRDSystemNames.values(requiredDevice))
		{
			QVariantMap config;
			config[CHardwareSDK::SystemName] = systemName;

			TNamedDevice required = createDevice(requiredDevice, config, true);

			if (!required.second)
			{
				continue;
			}

			// Запускаем асинхронный поиск.
			synchronizer.addFuture(QtConcurrent::run(this, &DeviceManager::findDevice, required.second, devicesToFind));
			requiredList.append(required.second);
		}
	}

	synchronizer.waitForFinished();
	mStopFlag = 1;

	// Формируем результат.
	int i = 0;

	foreach (QFuture<TNamedDevice> future, synchronizer.futures())
	{
		IDevice * device = future.result().second;

		if (device)
		{
			result.append(future.result().first);
		}
		else
		{
			// Уинчтожаем ресурс, т.к. на нем ничего не найдено.
			releaseDevice(requiredList[i]);
		}

		i++;
	}

	// Производим откат устройств, если требуется.
	foreach (const TNamedDevice & device, fallbackDevices)
	{
		if (!isDetected(device.first))
		{
			setDeviceLog(device.second, false);

			emit deviceDetected(device.first, device.second);

			result.append(device.first);
		}
	}

	return result;
}

//------------------------------------------------------------------------------
void DeviceManager::changeInstancePath(QString & aInstancePath, const QString & aConfigPath, const TPaths & aPaths)
{
	QString driverPath = aInstancePath.section(CPlugin::InstancePathSeparator, 0, 0);

	for (auto it = aPaths.begin(); it != aPaths.end(); ++it)
	{
		QString oldPath = it.key();

		if (driverPath.toLower() == oldPath.toLower())
		{
			for (auto jt = it->begin(); jt != it->end(); ++jt)
			{
				QString newPath = *jt;
				QVariantMap config = mPluginLoader->getPluginInstanceConfiguration(newPath, aConfigPath);
				SPluginParameter nameParameter = findParameter(CHardwareSDK::ModelName, getDriverParameters(newPath));

				if (nameParameter.isValid())
				{
					QString configProtocolName = config[CHardwareSDK::ProtocolName].toString();
					SPluginParameter protocolParameter = findParameter(CHardwareSDK::ProtocolName, getDriverParameters(newPath));

					bool protocolOK = configProtocolName.isEmpty();

					if (!protocolOK && protocolParameter.isValid())
					{
						QStringList possibleProtocolNames = protocolParameter.possibleValues.keys();

						if (!possibleProtocolNames.isEmpty())
						{
							protocolOK = possibleProtocolNames.contains(configProtocolName, Qt::CaseInsensitive);
						}
					}

					QStringList possibleModelNames = nameParameter.possibleValues.keys();
					QString configModelName = config[CHardwareSDK::ModelName].toString();

					if (possibleModelNames.contains(configModelName, Qt::CaseInsensitive) && protocolOK && (driverPath != newPath))
					{
						aInstancePath.replace(oldPath, newPath);

						return;
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
void DeviceManager::checkITInstancePath(QString & aInstancePath)
{
	QString configPath = aInstancePath;
	checkInstancePath(aInstancePath);

	TPaths paths;
	paths.insert("Common.Driver.FiscalRegistrator.COM.ATOL", TNewPaths()
	          << "Common.Driver.FiscalRegistrator.COM.ATOL"
	          << "Common.Driver.FiscalRegistrator.COM.ATOL.PayCTS2000"
	          << "Common.Driver.FiscalRegistrator.COM.ATOL.Single");
	paths.insert("Common.Driver.DocumentPrinter.COM.ATOL", TNewPaths()
	          << "Common.Driver.DocumentPrinter.COM.ATOL"
	          << "Common.Driver.DocumentPrinter.COM.ATOL.Single");
	paths.insert("Common.Driver.Printer.COM.POS.Custom", TNewPaths()
	          << "Common.Driver.Printer.COM.POS.Custom"
	          << "Common.Driver.Printer.COM.POS.CustomTG2480H");

	paths.insert("Common.Driver.BillAcceptor.COM", TNewPaths()
	          << "Common.Driver.BillAcceptor.COM.CCNet"
	          << "Common.Driver.BillAcceptor.COM.CCNet.CashcodeGX"
	          << "Common.Driver.BillAcceptor.COM.CCNet.Creator"
	          << "Common.Driver.BillAcceptor.COM.CCNet.Recycler"
	          << "Common.Driver.BillAcceptor.COM.EBDS"
	          << "Common.Driver.BillAcceptor.COM.ICT"
	          << "Common.Driver.BillAcceptor.COM.ID003"
	          << "Common.Driver.BillAcceptor.COM.V2e");

	paths.insert("Common.Driver.FiscalRegistrator.TCP.ShtrihOnline.Pay", TNewPaths()
	          << "Common.Driver.FiscalRegistrator.TCP.ShtrihOnline.Ejector");
	paths.insert("Common.Driver.FiscalRegistrator.COM.ShtrihOnline.Pay", TNewPaths()
	          << "Common.Driver.FiscalRegistrator.COM.ShtrihOnline.Ejector");

	changeInstancePath(aInstancePath, configPath, paths);

	paths.clear();
	paths.insert("Common.Driver.FiscalRegistrator.TCP.ShtrihOnline.Ejector", TNewPaths()
	          << "Common.Driver.FiscalRegistrator.TCP.ShtrihOnline.PayOnline"
	          << "Common.Driver.FiscalRegistrator.TCP.ShtrihOnline.PayVKP80FA");
	paths.insert("Common.Driver.FiscalRegistrator.COM.ShtrihOnline.Ejector", TNewPaths()
	          << "Common.Driver.FiscalRegistrator.COM.ShtrihOnline.PayOnline"
	          << "Common.Driver.FiscalRegistrator.COM.ShtrihOnline.PayVKP80FA");

	changeInstancePath(aInstancePath, configPath, paths);

	QStringList pathData = aInstancePath.split(".");

	if (pathData.size() > 4)
	{
		if (!pathData[4].compare("ATOL",       Qt::CaseInsensitive)) pathData[4] = pathData[4].replace("ATOL",       "ATOL2",       Qt::CaseInsensitive);
		if (!pathData[4].compare("AtolOnline", Qt::CaseInsensitive)) pathData[4] = pathData[4].replace("AtolOnline", "ATOL2Online", Qt::CaseInsensitive);

		aInstancePath = pathData.join(".");
	}
}

//------------------------------------------------------------------------------
void DeviceManager::checkInstancePath(QString & aInstancePath)
{
	QStringList pluginList = mPluginLoader->getPluginList(QRegExp("\\.Driver\\."));
	QString initialPath = aInstancePath.section(CPlugin::InstancePathSeparator, 0, 0);

	if (pluginList.contains(initialPath))
	{
		return;
	}

	RenamePluginPath renamePath;

	// в путях плгинов присутствует IT
	bool ITPluginPaths = std::find_if(pluginList.begin(), pluginList.end(), [&](const QString & aPath) -> bool
		{ return aPath.split(".").size() > 4; }) != pluginList.end();

	QStringList interactionTypes = QStringList()
		<< CInteractionTypes::COM
		<< CInteractionTypes::USB
		<< CInteractionTypes::TCP
		<< CInteractionTypes::OPOS
		<< CInteractionTypes::System
		<< CInteractionTypes::External;

	QStringList initialPathParts = initialPath.split(".");
	bool noITConfigPath = !(initialPathParts.size() > 4) && ((initialPathParts.size() < 4) || !interactionTypes.contains(initialPathParts[3]) || renamePath.contains(initialPath));

	QString extension = aInstancePath.section(CPlugin::InstancePathSeparator, 1, 1);

	if (ITPluginPaths && noITConfigPath && renamePath.contains(initialPath))
	{
		aInstancePath = renamePath[initialPath] + CPlugin::InstancePathSeparator + extension;

		return;
	}

	QStringList pathParts = initialPath.split(".");

	typedef QSet<QString> TPathParts;
	typedef QMap<QString, TPathParts> TPathDependences;
	typedef QPair<TPathParts, TPathDependences> TDeviceDependences;
	typedef QMap<QString, TDeviceDependences> TDependences;

	TPathDependences FRDependences;
	FRDependences.insert("Shtrih", TPathParts() << "CommonShtrih" << "RetractorShtrih" << "Shtrih-M Yarus-01K" << "Shtrih-M Shtrih Kiosk-FR-K");
	FRDependences.insert("ATOL",   TPathParts() << "CommonATOL"   << "PayVKP-80K"   << "PayPPU-700K" << "ATOL FR" << "ATOL DP");
	FRDependences.insert("Pay",    TPathParts() << "PayVKP-80K"   << "PayPPU-700K");
	FRDependences.insert("Sunphor", TPathParts() << "Sunphor POS58IV");
	TPathParts printerDeviceTypes = TPathParts()
		<< CComponents::FiscalRegistrator
		<< CComponents::DocumentPrinter;

	TPathDependences cameraDependences;
	cameraDependences.insert("DirectX", TPathParts() << "WebCamera");

	TPathDependences scannerDependences;
	scannerDependences.insert("Generic", TPathParts() << "USB Scanner");
	scannerDependences.insert("Proton",  TPathParts() << "Generic serial HID");

	TDependences dependences;
	dependences.insert(CComponents::Printer,           TDeviceDependences(printerDeviceTypes, FRDependences));
	dependences.insert(CComponents::FiscalRegistrator, TDeviceDependences(printerDeviceTypes, FRDependences));
	dependences.insert(CComponents::Camera,            TDeviceDependences(TPathParts() << CComponents::Camera, cameraDependences));
	dependences.insert(CComponents::Scanner,           TDeviceDependences(TPathParts() << CComponents::Scanner, scannerDependences));
	QSet<QString> paths;

	if (ITPluginPaths && noITConfigPath)
	{
		pluginList = renamePath.keys();
	}

	for (TDependences::iterator typeIt = dependences.begin(); typeIt != dependences.end(); ++typeIt)
	{
		if (pathParts[2].contains(typeIt.key(), Qt::CaseInsensitive))
		{
			foreach (const QString aDeviceType, typeIt->first)
			{
				pathParts[2] = aDeviceType;
				QString path = pathParts.join(".");

				if (pluginList.contains(path))
				{
					paths << path;
				}

				for (TPathDependences::iterator deviceIt = typeIt->second.begin(); deviceIt != typeIt->second.end(); ++deviceIt)
				{
					if (pathParts[3].contains(deviceIt.key(), Qt::CaseInsensitive))
					{
						foreach (const QString aDeviceExtention, deviceIt.value())
						{
							pathParts[3] = aDeviceExtention;
							path = pathParts.join(".");

							if (pluginList.contains(path))
							{
								paths << path;
							}
						}
					}
				}
			}
		}
	}

	if (paths.isEmpty())
	{
		return;
	}

	QSet<QString> resultPaths;

	foreach (const QString & instancePath, paths)
	{
		if (mAcquiredDevices.contains(instancePath))
		{
			resultPaths << instancePath;
		}
		else
		{
			QString fullInstancePath = instancePath + CPlugin::InstancePathSeparator + extension;
			IDevice * device = acquireDevice(fullInstancePath, aInstancePath);

			if (device)
			{
				// имя девайса могло измениться
				/*
				QString configDeviceName = mDeviceManager->getDeviceConfiguration(device)[CHardwareSDK::ModelName].toString();

				TParameterList parameterList = mDeviceManager->getDriverParameters(instancePath);
				TParameterList::iterator it = std::find_if(parameterList.begin(), parameterList.end(), [] (const SPluginParameter & aParameter) -> bool
					{ return aParameter.name == CHardwareSDK::ModelName; });

				if ((it != parameterList.end()) && (it->type == SPluginParameter::Set) && it->possibleValues.keys().contains(configDeviceName))
				*/
				{
					resultPaths << instancePath;
				}

				releaseDevice(device);
			}
		}
	}

	QString path = initialPath;

	if (resultPaths.size() == 1)
	{
		path = *resultPaths.begin();
	}
	else
	{
		//ищем сommon-путь; если найдется - используем его, не найдется - ничего не делаем
		if (!resultPaths.isEmpty())
		{
			paths = resultPaths;
		}

		QString commonPath;

		foreach (const QString aPath, paths)
		{
			if (aPath.split(".").last().toLower().contains("common"))
			{
				commonPath = aPath;

				break;
			}
		}

		if (!commonPath.isEmpty())
		{
			path = commonPath; 
		}
		else if (!resultPaths.isEmpty())
		{
			path = *resultPaths.begin();
		}
	}

	if (ITPluginPaths && noITConfigPath && renamePath.contains(path))
	{
		path = renamePath[path];
	}

	aInstancePath = path + CPlugin::InstancePathSeparator + extension;
}

//--------------------------------------------------------------------------------
void DeviceManager::stopDetection()
{
	mStopFlag = 1;
}

//--------------------------------------------------------------------------------
void DeviceManager::saveConfiguration(IDevice * aDevice)
{
	foreach (auto device, mDeviceDependencyMap.values(aDevice))
	{
		saveConfiguration(device);
	}

	dynamic_cast<IPlugin *>(aDevice)->saveConfiguration();
}

//--------------------------------------------------------------------------------
void DeviceManager::setDeviceConfiguration(IDevice * aDevice, const QVariantMap & aConfig)
{
	// Вычисляем старое и новое имя системного устройства
	QString oldSystemName;
	QString newSystemName;

	// Приверка свободности системного устройства 
	foreach (IDevice * systemDevice, mDeviceDependencyMap.values(aDevice))
	{
		// При изменении имени системного устройства меняем его доступность
		oldSystemName = systemDevice->getDeviceConfiguration()[CHardwareSDK::SystemName].toString();
		newSystemName = aConfig[CHardwareSDK::SystemName].toString();

		if (oldSystemName != newSystemName)
		{
			IPlugin * plugin = dynamic_cast<IPlugin *>(systemDevice);
			QString driverPath = plugin->getConfigurationName().section(CPlugin::InstancePathSeparator, 0, 0);

			if (mRDSystemNames.contains(driverPath) && !mFreeSystemNames.contains(newSystemName))
			{
				// Указанное системное имя недоступно.
				toLog(LogLevel::Error, QString("Required system name %1 is not available.").arg(newSystemName));

				return;
			}
		}
	}

	// Применяем конфигурацию к системным устройствам
	foreach (IDevice * systemDevice, mDeviceDependencyMap.values(aDevice))
	{
		// При изменении имени системного устройства меняем его доступность
		if (oldSystemName != newSystemName)
		{
			IPlugin * plugin = dynamic_cast<IPlugin *>(systemDevice);
			QString driverPath = plugin->getConfigurationName().section(CPlugin::InstancePathSeparator, 0, 0);

			if (mRDSystemNames.contains(driverPath))
			{
				mFreeSystemNames.insert(oldSystemName);
				mFreeSystemNames.remove(newSystemName);
			}
		}

		// Выставляем новую конфигурацию системному устройству
		systemDevice->setDeviceConfiguration(aConfig);
		setDeviceLog(aDevice, false);
	}

	// TODO: Не давать служебным полям попасть наверх.
	QVariantMap config = aConfig;
	config.remove(CHardwareSDK::RequiredResource);

	aDevice->setDeviceConfiguration(config);
}

//--------------------------------------------------------------------------------
QVariantMap DeviceManager::getDeviceConfiguration(IDevice * aDevice) const
{
	QVariantMap result = aDevice->getDeviceConfiguration();

	// Проверяем, чтобы соответсвующий параметр был в описании плагина.
	QString driverName = dynamic_cast<IPlugin *>(aDevice)->getConfigurationName().section(CPlugin::InstancePathSeparator, 0, 0);

	foreach (const QString & paramName, result.keys())
	{
		if (!findParameter(paramName, mDriverParameters[driverName]).isValid())
		{
			result.remove(paramName);
		}
	}

	foreach (auto device, mDeviceDependencyMap.values(aDevice))
	{
		auto parameters = getDeviceConfiguration(device);

		for (auto it = parameters.begin(); it != parameters.end(); ++it)
		{
			result.insert(it.key(), it.value());
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
TParameterList DeviceManager::getDriverParameters(const QString & aDriverPath) const
{
	TParameterList parameters = mDriverParameters[aDriverPath];

	if (mRequiredResources.contains(aDriverPath))
	{
		parameters << getDriverParameters(mRequiredResources[aDriverPath]);
	}

	return parameters;
}

//--------------------------------------------------------------------------------
QStringList DeviceManager::getDriverList() const
{
	return mRequiredResources.keys();
}

//--------------------------------------------------------------------------------
bool DeviceManager::isDetected(const QString & aConfigName)
{
	QMutexLocker lock(&mAccessMutex);

	QString deviceType = aConfigName.section('.', 2, 2);

	if (CComponents::isPrinter(deviceType))
	{
		foreach (auto type, mDetectedDeviceTypes)
		{
			if (CComponents::isPrinter(type))
			{
				return true;
			}
		}
	}

	return mDetectedDeviceTypes.contains(deviceType);
}

//--------------------------------------------------------------------------------
void DeviceManager::markDetected(const QString & aConfigName)
{
	QMutexLocker lock(&mAccessMutex);

	mDetectedDeviceTypes.insert(aConfigName.section('.', 2, 2));
}

//--------------------------------------------------------------------------------
bool DeviceManager::deviceSortPredicate(const QString & aLhs, const QString & aRhs) const
{
	// Берем приоритеты для конкретных моделей устройств.

	SPluginParameter parameter = findParameter(CHardwareSDK::DetectingPriority, mDriverParameters.value(aLhs));
	int lhsDevicePriority = parameter.isValid() ? parameter.defaultValue.value<int>() : DSDK::EDetectingPriority::Normal;

	parameter = findParameter(CHardwareSDK::DetectingPriority, mDriverParameters.value(aRhs));
	int rhsDevicePriority = parameter.isValid() ? parameter.defaultValue.value<int>() : DSDK::EDetectingPriority::Normal;

	// Производим сравнение.

	return lhsDevicePriority > rhsDevicePriority;
}

//--------------------------------------------------------------------------------
QString DeviceManager::getSimpleDeviceLogName(IDevice * aDevice, bool aDetecting)
{
	QList<int> logIndexes;
	int logIndex = 0;

	auto filterIndexes = [&logIndexes] () -> int { qSort(logIndexes);
		if (logIndexes.isEmpty() || logIndexes[0]) return 0;
		for (int i = 0; i < logIndexes.last(); ++i) if (!logIndexes.contains(i)) return i;

		return logIndexes.last() + 1;
	};

	QVariantMap parameters = aDevice->getDeviceConfiguration();
	QString interactionType = parameters[CHardwareSDK::InteractionType].toString();

	IPlugin * plugin = dynamic_cast<IPlugin *>(aDevice);
	QString configPath = plugin->getConfigurationName().section(CPlugin::InstancePathSeparator, 0, 0);

	if (interactionType.contains(CInteractionTypes::USB))
	{
		for (auto it = mDevicesLogData.begin(); it != mDevicesLogData.end(); ++it)
		{
			QVariantMap localParameters = it.key()->getDeviceConfiguration();
			QString localInteractionType = localParameters[CHardwareSDK::InteractionType].toString();

			if (localInteractionType.contains(CInteractionTypes::USB) && it->contains(aDetecting))
			{
				logIndexes << it->value(aDetecting);
			}
		}

		logIndex = filterIndexes();
	}
	else if (mDevicesLogData.contains(aDevice) && mDevicesLogData[aDevice].contains(aDetecting))
	{
		logIndex = mDevicesLogData[aDevice][aDetecting];
	}
	else
	{
		for (auto it = mDevicesLogData.begin(); it != mDevicesLogData.end(); ++it)
		{
			if (it->contains(aDetecting))
			{
				IPlugin * localPlugin = dynamic_cast<IPlugin *>(it.key());

				if (localPlugin->getConfigurationName().startsWith(configPath))
				{
					logIndexes << it->value(aDetecting);
				}
			}
		}

		logIndex = filterIndexes();
	}

	QString result = configPath.section('.', 2, 2);

	if (aDetecting)
	{
		result.prepend("autodetect/");
	}

	if (interactionType.contains(CInteractionTypes::USB))
	{
		result += QString(" on USB%1").arg(logIndex);
	}
	else if (interactionType == CInteractionTypes::TCP)
	{
		QVariantMap configuration = aDevice->getDeviceConfiguration();
		result += QString(" on %1 port %2")
			.arg(configuration[CHardwareSDK::Port::TCP::IP].toString())
			.arg(configuration[CHardwareSDK::Port::TCP::Number].toString());
	}
	else
	{
		result += QString(" %1").arg(logIndex);

		if (!interactionType.isEmpty())
		{
			result += QString(" on %1 driver").arg(interactionType);
		}
	}

	mDevicesLogData[aDevice].insert(aDetecting, logIndex);

	return result;
}

//--------------------------------------------------------------------------------
QString DeviceManager::getDeviceLogName(IDevice * aDevice, bool aDetecting)
{
	IPlugin * plugin = dynamic_cast<IPlugin *>(aDevice);
	QString configPath = plugin->getConfigurationName().section(CPlugin::InstancePathSeparator, 0, 0);
	QString result = configPath.section('.', 2, 2);
	int sections = configPath.split('.').size();

	if (aDetecting)
	{
		if (mRDSystemNames.contains(configPath))
		{
			QString systemName = aDevice->getDeviceConfiguration()[CHardwareSDK::SystemName].toString();
			result = "Port " + systemName;
		}
		else if (sections > 4)
		{
			result = configPath.section('.', 4, 4);
		}

		result.prepend("autodetect/");
	}

	IDevice * required = mDeviceDependencyMap.contains(aDevice) ? mDeviceDependencyMap.value(aDevice) : 0;

	if (required)
	{
		QString logData = required->getDeviceConfiguration()[CHardwareSDK::SystemName].toString();

		if (aDevice->getDeviceConfiguration()[CHardwareSDK::InteractionType].toString() == CInteractionTypes::TCP)
		{
			if (aDetecting)
			{
				result += QString(" on %1").arg(CInteractionTypes::TCP);
			}
			else
			{
				QVariantMap configuration = required->getDeviceConfiguration();
				result += QString(" on %1 port %2")
					.arg(configuration[CHardwareSDK::Port::TCP::IP].toString())
					.arg(configuration[CHardwareSDK::Port::TCP::Number].toString());
			}
		}
		else if (!logData.isEmpty())
		{
			result += " on " + logData;
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
void DeviceManager::setDeviceLog(IDevice * aDevice, bool aDetecting)
{
	IPlugin * plugin = dynamic_cast<IPlugin *>(aDevice);
	QString configPath = plugin->getConfigurationName().section(CPlugin::InstancePathSeparator, 0, 0);
	TParameterList parameters = mPluginLoader->getPluginParametersDescription(configPath);
	bool simpleDevice = !findParameter(CHardwareSDK::RequiredResource, parameters).isValid();

	QString logFileName = simpleDevice ? getSimpleDeviceLogName(aDevice, aDetecting) : getDeviceLogName(aDevice, aDetecting);
	ILog * log = ILog::getInstance(logFileName);

	if (mDeviceDependencyMap.contains(aDevice))
	{
		mDeviceDependencyMap.value(aDevice)->setLog(log);
	}

	aDevice->setLog(log);
}

//--------------------------------------------------------------------------------
