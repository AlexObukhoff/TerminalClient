/* @file Загрузчик плагинов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QRegExp>
#include <QtCore/QSharedPointer>
#include <QtCore/QPluginLoader>
#include <QtCore/QMutex>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/IKernel.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/Plugins/IPluginFactory.h>

namespace SDK {
namespace Plugin {

//------------------------------------------------------------------------------
/// Загрузчик плагинов.
class PluginLoader: public IPluginLoader
{
public:
	PluginLoader(IKernel * aKernel);
	virtual ~PluginLoader();

	#pragma region IPluginLoader interface

	/// Добавляет каталог с плагинами.
	virtual int addDirectory(const QString & aDirectory);

	/// Возвращает список доступных плагинов.
	virtual QStringList getPluginList(const QRegExp & aFilter) const;

	/// Возвращает список путей загруженных плагинов.
	virtual QStringList getPluginPathList(const QRegExp & aFilter) const;

	/// Создаёт плагин по заданному пути.
	virtual IPlugin * createPlugin(const QString & aInstancePath, const QString & aConfigPath = "");

	virtual TParameterList getPluginParametersDescription(const QString & aPath) const;

	/// Возвращает параметры для данного плагина, загруженные из конфигурационного файла или внешнего хранилища.
	virtual QVariantMap getPluginInstanceConfiguration(const QString & aInstancePath, const QString & aConfigPath);

	/// Удаляет плагин.
	virtual bool destroyPlugin(IPlugin * aPlugin);

	#pragma endregion

private:
	/// Интерфейс приложения для плагинов.
	IKernel * mKernel;

	/// Каталоги для поиска плагинов.
	QStringList mDirectories;

	/// Загрузчики библиотек. Нужны для выгрузки библиотек.
	QList<QSharedPointer<QPluginLoader> > mLibraries;

	/// Список доступных плагинов.
	QMap<QString, SDK::Plugin::IPluginFactory *> mPlugins;

	/// Список созданных плагинов.
	QMap<SDK::Plugin::IPlugin *, SDK::Plugin::IPluginFactory *> mCreatedPlugins;

	/// Синхронизация создания/удаления плагина.
	mutable QMutex mAccessMutex;
};

}} // namespace SDK::Plugin

//------------------------------------------------------------------------------
