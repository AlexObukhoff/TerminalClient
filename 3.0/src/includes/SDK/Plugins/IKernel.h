/* @file Интерфейс ядра приложения для плагинов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILog.h>

// SDK
#include "IExternalInterface.h"

namespace SDK {
namespace Plugin {

class IPluginLoader;

//------------------------------------------------------------------------------
/// Интерфейс ядра.
class IKernel
{
public:
	/// Возвращает логгер.
	virtual ILog * getLog(const QString & aName = "") const = 0;

	/// Возвращает версию ядра.
	virtual QString getVersion() const = 0;

	/// Возвращает рабочу папку ядра.
	virtual QString getDirectory() const = 0;

	/// Возвращает каталог для хранения данных/конфигов приложения.
	virtual QString getDataDirectory() const = 0;

	/// Возвращает каталог для хранения лог-файлов приложения.
	virtual QString getLogsDirectory() const = 0;

	/// Методы управления конфигурациями плагинов, хранимых приложением.
	/// Сообщает имеется ли для данной пары плагин-объект конфигурация.
	virtual bool canConfigurePlugin(const QString & aInstancePath) const = 0;

	/// Возвращает конфигурацию для данной пары плагин-объект.
	virtual QVariantMap getPluginConfiguration(const QString & aInstancePath) const = 0;

	/// Сообщает сможет ли сохранить конфигурацию для данной пары плагин-объект.
	virtual bool canSavePluginConfiguration(const QString & aInstancePath) const = 0;

	/// Сохраняет конфигурацию для данной пары плагин-объект.
	virtual bool savePluginConfiguration(const QString & aInstancePath, const QVariantMap & aParamenters) = 0;

	virtual IExternalInterface * getInterface(const QString & aInterface) = 0;

	/// Возвращает загрузчик плагинов.
	virtual IPluginLoader * getPluginLoader() const = 0;
};

//------------------------------------------------------------------------------
}} // namespace SDK::Plugin

//------------------------------------------------------------------------------
