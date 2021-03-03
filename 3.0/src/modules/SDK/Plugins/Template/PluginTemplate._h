/* @file Шаблон объявления плагина. */

#pragma once

// SDK
#include <Common/ILogable.h>

// Plugin SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IPluginFactory.h>

//------------------------------------------------------------------------------
class Plugin: public SDK::Plugin::IPlugin, public ILogable
{
public:
	Plugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath);

public:
	#pragma region SDK::Plugin::IPlugin interface

	/// IPlugin: Возвращает название плагина.
	virtual QString getPluginName() const;

	/// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
	virtual QString getConfigurationName() const;

	/// IPlugin: Возвращает параметры плагина.
	virtual QVariantMap getConfiguration() const;

	/// IPlugin: Настраивает плагин.
	virtual void setConfiguration(const QVariantMap & aConfiguration);

	/// IPlugin: Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной программы).
	virtual bool saveConfiguration();

	/// Проверяет успешно ли инициализировался плагин при создании.
	virtual bool isReady() const;

	#pragma endregion

private:
	SDK::Plugin::IEnvironment * mEnvironment;
	QString mInstancePath;
	QVariantMap mParameters;
};

//------------------------------------------------------------------------------
