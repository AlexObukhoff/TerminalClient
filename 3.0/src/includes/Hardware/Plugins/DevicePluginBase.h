/* @file Шаблонный плагин для драйверов. */

#pragma once

// Plugin SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/Plugins/IPluginLoader.h>

//------------------------------------------------------------------------------
template <class T>
class DevicePluginBase : public SDK::Plugin::IPlugin, public T
{
public:
	DevicePluginBase(const QString & aPluginName, SDK::Plugin::IEnvironment * aEnvironment, const QString & aInstancePath) : 
		mInstanceName(aInstancePath),
		mEnvironment(aEnvironment)
	{
		mPluginName = aPluginName + " plugin";
		setLog(aEnvironment->getLog(""));

		if (mEnvironment)
		{
			SDK::Plugin::IPluginLoader * pluginLoader = mEnvironment->getPluginLoader();

			if (pluginLoader)
			{
				QString path = mInstanceName.section(CPlugin::InstancePathSeparator, 0, 0);
				SDK::Plugin::TParameterList parameterList = pluginLoader->getPluginParametersDescription(path);
				QStringList pluginParameterNames;
				QStringList requiredResourceNames;

				foreach(const SDK::Plugin::SPluginParameter & parameter, parameterList)
				{
					if (parameter.name == CHardwareSDK::RequiredResource)
					{
						path = parameter.defaultValue.toString().section(CPlugin::InstancePathSeparator, 0, 0);
						SDK::Plugin::TParameterList rrParameterList = pluginLoader->getPluginParametersDescription(path);

						foreach(const SDK::Plugin::SPluginParameter & rrParameter, rrParameterList)
						{
							if (!rrParameter.readOnly)
							{
								requiredResourceNames << rrParameter.name;
							}
						}
					}
					else
					{
						pluginParameterNames << parameter.name;
					}
				}

				QVariantMap configuration;
				configuration.insert(CHardware::PluginParameterNames, pluginParameterNames);
				configuration.insert(CHardware::RequiredResourceNames, requiredResourceNames);
				configuration.insert(CHardware::PluginPath, makeDriverPath<T>());
				T::setDeviceConfiguration(configuration);
			}
		}
	}

	virtual ~DevicePluginBase() {}

	/// Возвращает название плагина.
	virtual QString getPluginName() const
	{
		return mPluginName;
	}

	/// Возвращает параметры плагина.
	virtual QVariantMap getConfiguration() const
	{
		return T::getDeviceConfiguration();
	}

	/// Настраивает плагин.
	virtual void setConfiguration(const QVariantMap & aParameters)
	{
		T::setDeviceConfiguration(aParameters);
	}

	/// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной программы).
	virtual bool saveConfiguration()
	{
		return mEnvironment->saveConfiguration(mInstanceName, T::getDeviceConfiguration());
	}

	/// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
	virtual QString getConfigurationName() const
	{
		return mInstanceName;
	}

	/// Проверяет успешно ли инициализировался плагин при создании.
	virtual bool isReady() const
	{
		return true;
	}

private:
	QString mPluginName;
	QString mInstanceName;
	SDK::Plugin::IEnvironment * mEnvironment;
};

//--------------------------------------------------------------------------------
