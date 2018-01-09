/* @file Интерфейс среды для использования из плагинов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILog.h>

namespace SDK {
namespace Plugin {

class IKernel;
class IPluginLoader;
class IExternalInterface;

//------------------------------------------------------------------------------
/// Интерфейс среды, в которой работают плагины.
class IEnvironment
{
public:
	/// Возвращает лог приложения.
	virtual ILog * getLog(const QString & aName) = 0;

	/// Возвращает версию ядра.
	virtual QString getKernelVersion() const = 0;

	/// Возвращает рабочую папку ядра.
	virtual QString getKernelDirectory() const = 0;

	/// Возвращает папку с конфигурацией ядра.
	virtual QString getKernelDataDirectory() const = 0;

	/// Возвращает каталог для хранения лог-файлов приложения.
	virtual QString getKernelLogsDirectory() const = 0;

	/// Возвращает папку, в которой находится плагин.
	virtual const QString & getPluginDirectory() const = 0;

	/// Сохраняет параметры данного плагина во внешнее хранилище. Используется плагинами.
	virtual bool saveConfiguration(const QString & aInstancePath, const QVariantMap & aParameters) = 0;

	/// Возвращает ядро расширяемого приложения.
	virtual IExternalInterface * getInterface(const QString & aInterface) = 0;

	/// Возвращает связанный загрузчик.
	virtual IPluginLoader * getPluginLoader() const = 0;
};

//------------------------------------------------------------------------------
}} // namespace SDK::Plugin

//------------------------------------------------------------------------------
