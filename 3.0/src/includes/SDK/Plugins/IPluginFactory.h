/* @file Интерфейс библиотеки плагинов для использования приложением. */

#pragma once

// std
#include <memory>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/IKernel.h>
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/PluginParameters.h>

namespace SDK {
namespace Plugin {

//------------------------------------------------------------------------------
/// Интерфейс фабрики плагинов.
class IPluginFactory
{
public:
	/// Инициализирует библиотеку.
	virtual bool initialize(IKernel * aKernel, const QString & aDirectory) = 0;

	/// Завершает работу библиотеки.
	virtual void shutdown() = 0;

	/// Возвращает название библиотеки.
	virtual QString getName() const = 0;

	/// Возвращает описание библиотеки.
	virtual QString getDescription() const = 0;

	/// Возвращает имя автора библиотеки.
	virtual QString getAuthor() const = 0;

	/// Возвращает версию библиотеки.
	virtual QString getVersion() const = 0;

	/// Возвращает список плагинов.
	virtual QStringList getPluginList() const = 0;

	/// Возвращает описание параметров плагина.
	virtual TParameterList getPluginParametersDescription(const QString & aPath) const = 0;

	/// Возвращает список работающих в данный момнет объектов.
	virtual QStringList getRuntimeConfigurations(const QString & aPathFilter) const = 0;

	/// Возвращает список сохранённых конфигураций.
	virtual QStringList getPersistentConfigurations(const QString & aPathFilter) const = 0;

	/// Возвращает параметры для данного плагина, загруженные из конфигурационного файла или внешнего хранилища.
	virtual QVariantMap getPluginInstanceConfiguration(const QString & aPath, const QString & aInstance) = 0;

	/// Создаёт плагин.
	virtual IPlugin * createPlugin(const QString & aInstancePath, const QString & aConfigPath) = 0;

	/// Создаёт плагин.
	virtual std::weak_ptr<IPlugin> createPluginPtr(const QString & aInstancePath, const QString & aConfigPath) = 0;

	/// Удаляет плагин.
	virtual bool destroyPlugin(IPlugin * aPlugin) = 0;

	/// Удаляет плагин.
	virtual bool destroyPlugin(const std::weak_ptr<IPlugin> & aPlugin) = 0;

	/// Локализует названия и описания параметров.
	virtual void translateParameters() = 0;
};

//------------------------------------------------------------------------------
}} // namespace SDK::Plugin

// Объявление интерфейса, доступного из библиотеки.
Q_DECLARE_INTERFACE(SDK::Plugin::IPluginFactory, "Cyberplat.*.System.PluginFactory")

//------------------------------------------------------------------------------
