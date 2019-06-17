#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QSharedPointer>
#include <QtCore/QFutureWatcher>
#include <QtCore/QReadWriteLock>
#include <Common/QtHeadersEnd.h>

// Modules
#include <ScenarioEngine/Scenario.h>

// Plugin SDK
#include <SDK/Plugins/IFactory.h>

#include "MainScenario.h"

class IApplication;

//--------------------------------------------------------------------------
namespace SDK {
	namespace PaymentProcessor {
		class ICore;
		class IPaymentService;
		class IPrinterService;
		class IHIDService;
		class ISettingsService;
	}

	namespace Plugin {
		class IEnvironment;
	}
}

namespace PPSDK = SDK::PaymentProcessor;

namespace CScenarioPlugin
{
	const QString PluginName = "Migrator3000";
}

namespace Migrator3000
{

//--------------------------------------------------------------------------
class MainScenarioPlugin : public SDK::Plugin::IFactory < GUI::Scenario >
{
public:
	MainScenarioPlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath) : mEnvironment(aFactory), mInstancePath(aInstancePath) {}

public:
	/// Возвращает название плагина.
	virtual QString getPluginName() const { return CScenarioPlugin::PluginName; }

	/// Возвращает параметры плагина.
	virtual QVariantMap getConfiguration() const
	{
		QVariantMap parameters;
		parameters["url"] = mUrl;
		return parameters;
	}

	/// Настраивает плагин.
	virtual void setConfiguration(const QVariantMap & aParameters)
	{
		QString url = aParameters["url"].toString();
		mUrl = url.isEmpty() ? mUrl : url;
	}

	/// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
	virtual QString getConfigurationName() const { return mInstancePath; }

	/// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной программы).
	virtual bool saveConfiguration() { return mEnvironment->saveConfiguration(CScenarioPlugin::PluginName, getConfiguration());	}

	/// Проверяет успешно ли инициализировался плагин при создании.
	virtual bool isReady() const { return true; }

	/// Возвращает список имен классов, которые создает фабрика.
	virtual QStringList getClassNames() const { return QStringList(CScenarioPlugin::PluginName); }

	/// Создает класс c заданным именем.
	virtual GUI::Scenario * create(const QString & aClassName) const
	{
		PPSDK::ICore * core = dynamic_cast<SDK::PaymentProcessor::ICore *>(mEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
		return new MainScenario(core, mEnvironment->getLog(aClassName));
	}

private:
	QString mUrl;
	QString mInstancePath;
	SDK::Plugin::IEnvironment * mEnvironment;
};

}