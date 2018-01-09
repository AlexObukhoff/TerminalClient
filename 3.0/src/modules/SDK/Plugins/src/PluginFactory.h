/* @file Объявление фабрики плагинов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QVariantMap>
#include <QtCore/QtPlugin>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/IPluginFactory.h>

namespace SDK {
namespace Plugin {

//------------------------------------------------------------------------------
/// Фабрика плагинов регистрирует плагины, слинкованные в данной динамической библиотеке и предоставляет
/// внешнему приложению их список и возможность создания. В одной библиотеке может быть только одна фабрика.

class PluginFactory : public QObject, public IPluginFactory, public IEnvironment
{
	Q_OBJECT
	Q_INTERFACES(SDK::Plugin::IPluginFactory)

public:
	PluginFactory();
	~PluginFactory();

	#pragma region IPluginFactory interface

	/// Инициализирует библиотеку.
	virtual bool initialize(IKernel * aKernel, const QString & aDirectory);

	/// Завершает работу библиотеки.
	virtual void shutdown();

	/// Возвращает название плагина.
	virtual QString getName() const;

	/// Возвращает описание плагина.
	virtual QString getDescription() const;

	/// Возвращает имя автора плагина.
	virtual QString getAuthor() const;

	/// Возвращает версию плагина.
	virtual QString getVersion() const;

	/// Возвращает список плагинов.
	virtual QStringList getPluginList() const;

	/// Создаёт плагин.
	virtual IPlugin * createPlugin(const QString & aInstancePath, const QString & aConfigPath);

	/// Удаляет плагин.
	virtual bool destroyPlugin(IPlugin * aPlugin);

	/// Возвращает описание параметров плагина.
	virtual TParameterList getPluginParametersDescription(const QString & aPath) const;

	/// Возвращает список работающих в данный момент плагинов.
	virtual QStringList getRuntimeConfigurations(const QString & aPathFilter) const;

	/// Возвращает список сохранённых конфигураций.
	virtual QStringList getPersistentConfigurations(const QString & aPathFilter) const;

	/// Сохраняет параметры данного плагина во внешнее хранилище. Используется плагинами.
	virtual bool saveConfiguration(const QString & aInstancePath, const QVariantMap & aParameters);

	#pragma endregion

	/// Методы интерфейса SDK::Plugin::IEnvironment
	/// Возвращает лог приложения.
	virtual ILog * getLog(const QString & aName);

	/// Возвращает версию ядра.
	virtual QString getKernelVersion() const;

	/// Возвращает рабочую папку ядра.
	virtual QString getKernelDirectory() const;

	/// Возвращает папку с конфигурацией ядра.
	virtual QString getKernelDataDirectory() const;

	/// Возвращает каталог для хранения лог-файлов приложения.
	virtual QString getKernelLogsDirectory() const;

	/// Возвращает папку, в которой находится плагин.
	virtual const QString & getPluginDirectory() const;

	/// Возвращает ядро расширяемого приложения.
	virtual IExternalInterface * getInterface(const QString & aInterface);

	/// Возвращает связанный загрузчик.
	virtual IPluginLoader * getPluginLoader() const;

	/// Локализует названия и описания параметров.
	virtual void translateParameters();

	/// Возвращает параметры для данного плагина, загруженные из конфигурационного файла или внешнего хранилища.
	virtual QVariantMap getPluginInstanceConfiguration(const QString & aPath, const QString & aInstance);

protected:
	static QString mModuleName;
	static QString mName;
	static QString mDescription;
	static QString mAuthor;
	static QString mVersion;

	bool mInitialized;
	IKernel * mKernel;
	QString mDirectory;
	QMap<IPlugin *, QString> mCreatedPlugins;
	QMap<QString, QVariantMap> mPersistentConfigurations;
	QMap<QString, TParameterList> mTranslatedParameters;
};

//------------------------------------------------------------------------------
}} // namespace SDK::Plugin

//------------------------------------------------------------------------------
