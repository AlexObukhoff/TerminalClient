/* @file Реализация фабрики плагинов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QSet>
#include <QtCore/QSettings>
#include <QtCore/QUuid>
#include <QtCore/QCoreApplication>
#include <Common/QtHeadersEnd.h>

// Common
#include <Common/Version.h>
#include <Common/ExceptionFilter.h>
#include <Common/PluginConstants.h>

// Plugin SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "PluginLoader.h"

namespace SDK {
namespace Plugin {

//------------------------------------------------------------------------------
/// Список имен параметров общих для любого плагина.
namespace Parameters {
	const char * Debug = "debug"; /// Режим отладки
	const char * Singleton = "singleton"; /// Плагин - единоличник
}

//------------------------------------------------------------------------------
namespace CPluginFactory
{
	/// Папка для сохранения конфигураций плагинов.
	const char ConfigurationDirectory[] = "plugins";
}

//------------------------------------------------------------------------------
PluginFactory::PluginFactory(): mKernel(0), mInitialized(false)
{
}

//------------------------------------------------------------------------------
PluginFactory::~PluginFactory()
{
	if (mInitialized)
	{
		shutdown();
	}
}

//------------------------------------------------------------------------------
bool PluginFactory::initialize(IKernel * aKernel, const QString & aDirectory)
{
	mInitialized = false;

	if (!aKernel)
	{
		Q_ASSERT(aKernel);
		return mInitialized;
	}

	mKernel = aKernel;
	mDirectory = aDirectory;

	mKernel->getLog()->write(LogLevel::Normal, QString("Initializing plugin library \"%1\". Core: %2.").arg(getName()).arg(Cyberplat::getVersion()));

	if (mKernel->getVersion() != Cyberplat::getVersion())
	{
		mKernel->getLog()->write(LogLevel::Warning, QString("Plugin library \"%1\" compiled with a different core versions. Compiled:[%2] Current core:[%3]")
			.arg(getName())
			.arg(Cyberplat::getVersion())
			.arg(mKernel->getVersion()));
	}

	// Отключение плагинов с пустым путём или конструктором
	for (PluginInitializer::TPluginList::iterator i = PluginInitializer::getPluginList().begin(); i != PluginInitializer::getPluginList().end(); ++i)
	{
		if (i.key().isEmpty() || !i.value().first)
		{
			mKernel->getLog()->write(LogLevel::Warning, QString("Disabling broken plugin %1: no path or constructor.").arg(i.key()));
		}
	}

	// Загрузка конфигураций (в одном файле хранятся ВСЕ конфигурации ВСЕХ плагинов данной библиотеки)
	QFileInfo file(QDir::toNativeSeparators(QDir::cleanPath(mKernel->getDataDirectory() + QDir::separator() +
		CPluginFactory::ConfigurationDirectory + QDir::separator() + mModuleName + ".ini")));

	if (file.exists())
	{
		mKernel->getLog()->write(LogLevel::Normal,
			QString("Configuration file %1 found, loading.").arg(file.absoluteFilePath()));

		QSettings config(file.absoluteFilePath(), QSettings::IniFormat);

		config.setIniCodec("utf-8");

		foreach (QString group, config.childGroups())
		{
			QVariantMap parameters;
			config.beginGroup(group);

			foreach (QString key, config.allKeys())
			{
				parameters[key] = config.value(key);
			}

			config.endGroup();
			mPersistentConfigurations[group] = parameters;
		}
	}

	mInitialized = true;

    return mInitialized;
}

//------------------------------------------------------------------------------
void PluginFactory::shutdown()
{
	if (mKernel)
	{
		mKernel->getLog()->write(LogLevel::Normal, QString("Shutting down plugin library \"%1\".").arg(getName()));

		foreach (IPlugin * plugin, mCreatedPlugins.keys())
		{
			delete plugin;
		}
	}

	mCreatedPlugins.clear();
	mInitialized = false;
}

//------------------------------------------------------------------------------
QString PluginFactory::getName() const
{
	return mName;
}

//------------------------------------------------------------------------------
QString PluginFactory::getDescription() const
{
    return mDescription;
}

//------------------------------------------------------------------------------
QString PluginFactory::getAuthor() const
{
    return mAuthor;
}

//------------------------------------------------------------------------------
QString PluginFactory::getVersion() const
{
    return mVersion;
}

//------------------------------------------------------------------------------
QStringList PluginFactory::getPluginList() const
{
    return PluginInitializer::getPluginList().keys();
}

//------------------------------------------------------------------------------
IPlugin * PluginFactory::createPlugin(const QString & aInstancePath, const QString & aConfigInstancePath)
{
	mKernel->getLog()->write(LogLevel::Normal, QString("Creating plugin %1.").arg(aInstancePath));

	IPlugin * plugin = 0;

	// Разделим на путь и конфигурацию
	QString path = aInstancePath.section(CPlugin::InstancePathSeparator, 0, 0);
	QString instance = aInstancePath.section(CPlugin::InstancePathSeparator, 1, 1);

	// Найдём нужный класс
	PluginInitializer::TPluginList::iterator pluginInfo = PluginInitializer::getPluginList().find(path);

	// Сгенерируем имя конфигурации, если отсутствует
	if (instance.isEmpty())
	{
		auto singleton = findParameter(SDK::Plugin::Parameters::Singleton, pluginInfo->second);

		if (singleton.isValid() && singleton.defaultValue.toBool())
		{
			instance = SDK::Plugin::Parameters::Singleton;
		}
		else
		{
			instance = QUuid::createUuid().toString();
			instance.remove('{');
			instance.remove('}');
		}
	}

	if (pluginInfo != PluginInitializer::getPluginList().end())
	{
		try {
			QString realInstancePath(path + CPlugin::InstancePathSeparator + instance);

			auto pluginConstructor = pluginInfo.value().first;
			plugin = pluginConstructor(this, realInstancePath);

			if (plugin && plugin->isReady())
			{
				// Инициализируем и конфигурируем плагин
				mCreatedPlugins[plugin] = realInstancePath;
				QString configPath     = aConfigInstancePath.section(CPlugin::InstancePathSeparator, 0, 0);
				QString configInstance = aConfigInstancePath.section(CPlugin::InstancePathSeparator, 1, 1);

				QVariantMap configuration = getPluginInstanceConfiguration(configPath, configInstance.isEmpty() ? instance : configInstance);

				// Синхронизируем настройки конфига и плагина, если плагин новее, чем конфиг
				TParameterList & parameterList = mTranslatedParameters[path];

				SPluginParameter modifiedKeys = findParameter(CPlugin::ModifiedKeys, parameterList);

				if (modifiedKeys.isValid())
				{
					for (auto it = modifiedKeys.possibleValues.begin(); it != modifiedKeys.possibleValues.end(); ++it)
					{
						QString key = it.key();
						QString newKey = it.value().toString();

						if (configuration.contains(key) && (!configuration.contains(newKey) || configuration[newKey].isNull() || !configuration[newKey].isValid()))
						{
							QVariant value = configuration[key];
							configuration.remove(key);
							configuration.insert(newKey, value);
						}
					}
				}

				IPluginLoader * pluginLoader = getPluginLoader();

				for (auto it = parameterList.begin(); it != parameterList.end(); ++it)
				{
					bool isSet = !it->possibleValues.isEmpty();

					for (auto jt = it->possibleValues.begin(); jt != it->possibleValues.end(); ++jt)
					{
						isSet = isSet && (jt.key() == jt.value());
					}

					bool notContains = !configuration.contains(it->name);
					QVariant configValue = configuration[it->name];

					if (notContains || (isSet && !it->defaultValue.toString().isEmpty() && !it->possibleValues.values().contains(configValue)))
					{
						QString newValue = it->defaultValue.toString();

						if (pluginLoader && !notContains && (it->name != CPlugin::ModifiedValues))
						{
							for (auto jt = parameterList.begin(); jt != parameterList.end(); ++jt)
							{
								if (jt->isValid() && (jt->name == CPlugin::ModifiedValues) && (jt->title.isEmpty() || (jt->title == it->name)))
								{
									QString modifiedValue = jt->possibleValues.value(configValue.toString()).toString();

									if (!modifiedValue.isEmpty())
									{
										newValue = modifiedValue;
									}
								}
							}
						}

						configuration.insert(it->name, newValue);
					}
				}

				/*
				//в случае изменения пути плагина с одновременным изменением настроек появятся 2 разные конфигурации одного устройства
				if (oldConfiguration != configuration)
				{
					QString realConfigPath(configPath + CPlugin::InstancePathSeparator + configInstance);
					saveConfiguration(realInstancePath, configuration);
				}
				*/

				// Добаляем версию PP и устанавливаем настройки плагина
				configuration.insert(CPluginParameters::PPVersion, mKernel->getVersion());
				plugin->setConfiguration(configuration);
			}
			else
			{
				mKernel->getLog()->write(LogLevel::Error, QString("Failed to create plugin %1.").arg(path));

				if (plugin)
				{
					delete plugin;
					plugin = nullptr;
				}
			}
		}
		catch (...)
		{
			EXCEPTION_FILTER_NO_THROW(mKernel->getLog());

			if (plugin)
			{
				delete plugin;
				plugin = nullptr;
			}
		}
	}
	else
	{
		mKernel->getLog()->write(LogLevel::Error, QString("No plugin found for %1.").arg(aInstancePath));
	}

	return plugin;
}

//------------------------------------------------------------------------------
bool PluginFactory::destroyPlugin(IPlugin * aPlugin)
{
	Q_ASSERT(aPlugin);

	if (aPlugin)
	{
		mKernel->getLog()->write(LogLevel::Normal, QString("Destroying plugin \"%1\".").arg(aPlugin->getPluginName()));

		if (mCreatedPlugins.contains(aPlugin))
		{
			mCreatedPlugins.remove(aPlugin);
			delete aPlugin;

			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
void PluginFactory::translateParameters()
{
	auto pluginList = PluginInitializer::getPluginList();

	foreach (auto pluginPath, pluginList.keys())
	{
		auto parameters = pluginList.value(pluginPath).second;
		auto pluginName = pluginPath.section(".", -1, -1).toLatin1();

		TParameterList result;

		auto translateItem = [&] (QString & aItem)
		{
			QStringList elements = aItem.split(" ");

			for (int i = 0; i < elements.size(); ++i)
			{
				// Контекст для перевода может содержаться в самой строке с переводом.
				auto context = elements[i].section("#", 0, 0).toLatin1();
				elements[i] = qApp->translate(context.isEmpty() ? pluginName : context, elements[i].toLatin1());
			}

			aItem = elements.join(" ");
		};

		// Производим локализацию названий и описаний параметров.
		foreach (auto parameter, parameters)
		{
			translateItem(parameter.title);
			translateItem(parameter.description);

			result.append(parameter);
		}

		mTranslatedParameters.insert(pluginPath, result);
	}
}

//------------------------------------------------------------------------------
IExternalInterface * PluginFactory::getInterface(const QString & aInterface)
{
	return mKernel->getInterface(aInterface);
}

//------------------------------------------------------------------------------
QString PluginFactory::getKernelVersion() const
{
	return mKernel->getVersion();
}

//------------------------------------------------------------------------------
QString PluginFactory::getKernelDirectory() const
{
	return mKernel->getDirectory();
}

//------------------------------------------------------------------------------
QString PluginFactory::getKernelDataDirectory() const
{
	return mKernel->getDataDirectory();
}

//------------------------------------------------------------------------------
QString PluginFactory::getKernelLogsDirectory() const
{
	return mKernel->getLogsDirectory();
}

//------------------------------------------------------------------------------
const QString & PluginFactory::getPluginDirectory() const
{
	return mDirectory;
}

//------------------------------------------------------------------------------
ILog * PluginFactory::getLog(const QString & aName)
{
	return mKernel->getLog(aName);
}

//------------------------------------------------------------------------------
TParameterList PluginFactory::getPluginParametersDescription(const QString & aPath) const
{
	PluginInitializer::TPluginList::Iterator plugin = PluginInitializer::getPluginList().find(aPath);

	if (plugin != PluginInitializer::getPluginList().end())
	{
		return mTranslatedParameters.contains(aPath) ? mTranslatedParameters.value(aPath) : plugin->second;
	}

	return TParameterList();
}

//------------------------------------------------------------------------------
QStringList PluginFactory::getRuntimeConfigurations(const QString & aPathFilter) const
{
	// Немного опимизации
	if (aPathFilter.isEmpty())
	{
		return mCreatedPlugins.values();
	}
	else
	{
		return QStringList(mCreatedPlugins.values()).filter(QRegExp(QString("^%1").arg(aPathFilter)));
	}
}

//------------------------------------------------------------------------------
QStringList PluginFactory::getPersistentConfigurations(const QString & aPathFilter) const
{
	// Немного опимизации
	if (aPathFilter.isEmpty())
	{
		return mPersistentConfigurations.keys();
	}
	else
	{
		return QStringList(mPersistentConfigurations.keys()).filter(QRegExp(QString("^%1").arg(aPathFilter)));
	}
}

//------------------------------------------------------------------------------
bool isContainContent(const QVariantMap & aStorage, const QVariantMap & aContent)
{
	foreach (auto key, aContent.keys())
	{
		if (!aStorage.contains(key) || aStorage.value(key) != aContent.value(key))
		{
			return false;
		}
	}

	return true;
}

//------------------------------------------------------------------------------
bool PluginFactory::saveConfiguration(const QString & aInstancePath, const QVariantMap & aParameters)
{
	bool result = false;

	// Сохраняем во внешнем хранилище, если можно
	if (mKernel->canSavePluginConfiguration(aInstancePath))
	{
		mKernel->getLog()->write(LogLevel::Error, QString("Saving configuration %1 to external storage.").arg(aInstancePath));

		// Фильтруем параметры - сохраняем только те, которые есть в описании плагина.
		TParameterList parameterDefinitions = getPluginParametersDescription(aInstancePath.section(CPlugin::InstancePathSeparator, 0, 0));
		QVariantMap parameters;

		foreach (QString key, aParameters.keys())
		{
			if (findParameter(key, parameterDefinitions).isValid() && aParameters[key].isValid())
			{
				parameters.insert(key, aParameters[key]);
			}
		}

		result = mKernel->savePluginConfiguration(aInstancePath, parameters);
	}
	else
	{
		// Формируем список параметров для сохранения в конфиг
		QVariantMap saveParameters;
		TParameterList parameters = getPluginParametersDescription(aInstancePath.section(CPlugin::InstancePathSeparator, 0, 0));

		foreach(QString key, aParameters.keys())
		{
			if (findParameter(key, parameters).isValid() && aParameters[key].isValid())
			{
				saveParameters[key] = aParameters[key];
			}
		}

		// Проверяем, нужно ли сохранять
		if (isContainContent(mPersistentConfigurations[aInstancePath], saveParameters))
		{
			mKernel->getLog()->write(LogLevel::Normal, QString("Skip saving configuration %1. Nothing changes.").arg(aInstancePath));
		}
		else
		{
			// Иначе в конфигурационный файл
			QString fileName = mKernel->getDataDirectory() + QDir::separator() + CPluginFactory::ConfigurationDirectory + QDir::separator() + mModuleName + ".ini";
			QSettings config(QDir::toNativeSeparators(QDir::cleanPath(fileName)), QSettings::IniFormat);

			config.setIniCodec("utf-8");

			result = config.isWritable();

			if (result)
			{
				mKernel->getLog()->write(LogLevel::Normal, QString("Saving configuration %1 to file.").arg(config.fileName()));

				config.beginGroup(aInstancePath);

				foreach (QString key, saveParameters.keys())
				{
					config.setValue(key, saveParameters[key]);
				}

				config.endGroup();
				config.sync();

				mPersistentConfigurations[aInstancePath] = aParameters;
			}
			else
			{
				mKernel->getLog()->write(LogLevel::Error,
					QString("Failed to save configuration %1: %2 is not writable.").arg(aInstancePath).arg(config.fileName()));
			}
		}
	}

	return result;
}

//------------------------------------------------------------------------------
QVariantMap PluginFactory::getPluginInstanceConfiguration(const QString & aPath, const QString & aInstance)
{
	QString instancePath = aPath + CPlugin::InstancePathSeparator + aInstance;

	// Если конфигурация есть у приложения, берём её
	if (mKernel->canConfigurePlugin(instancePath))
	{
		return mKernel->getPluginConfiguration(instancePath);
	}
	else
	{
		// Иначе из конфигурационного файла
		if (mPersistentConfigurations.contains(instancePath))
		{
			return mPersistentConfigurations[instancePath];
		}
		else if (mPersistentConfigurations.contains(aPath))
		{
			// Иначе из конфигурационного файла конфигурацию по умолчанию
			return mPersistentConfigurations[aPath];
		}
	}

	// Иначе пустую конфигурацию
	return QVariantMap();
}

//------------------------------------------------------------------------------
IPluginLoader * PluginFactory::getPluginLoader() const
{
	return mKernel->getPluginLoader();
}

//------------------------------------------------------------------------------
}} // namespace SDK::Plugin
