/* @file Базовый плагин. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IPluginEnvironment.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/PluginInitializer.h>

//------------------------------------------------------------------------------
template <class T>
class PluginBase : public SDK::Plugin::IPlugin, public T
{
public:
	PluginBase(const QString & aName, SDK::Plugin::IEnvironment * aEnvironment, const QString & aInstancePath) : 
		mInstanceName(aInstancePath),
		mEnvironment(aEnvironment),
		mPluginName(aName)
	{
		setLog(aEnvironment->getLog(aName));
	}

	virtual ~PluginBase() {}

	/// Возвращает название плагина.
	virtual QString getPluginName() const
	{
		return mPluginName;
	}

	/// Возвращает параметры плагина.
	virtual QVariantMap getConfiguration() const
	{
		return QVariantMap();
	}

	/// Настраивает плагин.
	virtual void setConfiguration(const QVariantMap & /*aParameters*/) {}

	/// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной программы).
	virtual bool saveConfiguration()
	{
		return true;
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
