/* @file Класс для регистрации плагина в фабрике. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QVector>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IPluginEnvironment.h>
#include <SDK/Plugins/PluginParameters.h>

namespace SDK {
namespace Plugin {

//------------------------------------------------------------------------------
/// Список плагинов в библиотеке. Каждая библиотека плагинов создаёт глобальный экземпляр
/// данного класса, который сохраняет путь к плагину в статическом списке. Затем этот список
/// используется фабрикой.

class PluginInitializer
{
public:
	/// Конструктор плагина.
	typedef IPlugin * (* TConstructor)(IEnvironment *, const QString &);

	/// Список плагинов: имя -> конструктор, параметры.
	typedef QMap<QString, QPair<TConstructor, TParameterList> > TPluginList;

	/// Конструктор, добавляет плагин в глобальный список, доступный фабрике.
	template <typename ParameterEnumerator>
	PluginInitializer(QString aPath, TConstructor aConstructor, ParameterEnumerator aParameterEnumerator)
	{
		getPluginList()[aPath] = qMakePair(aConstructor, aParameterEnumerator());
	}

	template <typename ParameterEnumerator>
	PluginInitializer * addPlugin(QString aPath, TConstructor aConstructor, ParameterEnumerator aParameterEnumerator)
	{
		getPluginList()[aPath] = qMakePair(aConstructor, aParameterEnumerator());

		return 0;
	}

	/// Функция доступа к статическому списку плагинов.
	static TPluginList & getPluginList()
	{
		static TPluginList pluginList;
		return pluginList;
	}

	static QVector<SPluginParameter> emptyParameterList()
	{
		return QVector<SPluginParameter>();
	}
};

/// Вспомогательные методы для формирования пути плагина.
namespace { QString makePath(const QString & aApplication, const QString & aComponent, const QString & aName)
{
	return QString("%1.%2.%3").arg(aApplication).arg(aComponent).arg(aName);
}}

namespace { QString makePath(const QString & aApplication, const QString & aComponent, const QString & aName, const QString & aExtension)
{
	return QString("%1.%2.%3.%4").arg(aApplication).arg(aComponent).arg(aName).arg(aExtension);
}}

/// Макрос для использования в коде плагина, регистрирует плагин без параметров в глобальном списке плагинов.
#define REGISTER_PLUGIN(aPath, aConstructor) \
	namespace { SDK::Plugin::PluginInitializer gPluginInitializer(aPath, aConstructor, &SDK::Plugin::PluginInitializer::emptyParameterList); }

/// Макрос для использования в коде плагина, регистрирует плагин с параметрами в глобальном списке плагинов.
#define REGISTER_PLUGIN_WITH_PARAMETERS(aPath, aConstructor, aParameterEnumerator) \
	namespace { SDK::Plugin::PluginInitializer gPluginInitializer(aPath, aConstructor, aParameterEnumerator); }

/// Набор макросов для описания нескольких плагинов в одном CPP файле.
#define BEGIN_REGISTER_PLUGIN \
	SDK::Plugin::PluginInitializer PluginArray[] = {

#define PLUGIN(aPath, aConstructor) \
	SDK::Plugin::PluginInitializer(aPath, aConstructor, &SDK::Plugin::PluginInitializer::emptyParameterList),

#define PLUGIN_WITH_PARAMETERS(aPath, aConstructor, aParameterEnumerator) \
	SDK::Plugin::PluginInitializer(aPath, aConstructor, aParameterEnumerator),

#define END_REGISTER_PLUGIN };

//------------------------------------------------------------------------------
}} // namespace SDK::Plugin

//------------------------------------------------------------------------------
