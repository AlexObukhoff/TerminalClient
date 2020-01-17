/* @file Плагин сценария для оплаты картами */
#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QSharedPointer>
#include <QtCore/QTimer>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/IFactory.h>
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IExternalInterface.h>

// PP SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Scripting/IBackendScenarioObject.h>
#include <SDK/PaymentProcessor/Components.h>

// Modules
#include <Common/ILogable.h>

// Project
#include "Ucs.h"


class IApplication;

//--------------------------------------------------------------------------
namespace SDK {
	namespace PaymentProcessor {
		class ICore;
	}

	namespace Plugin {
		class IEnvironment;
	}
}

namespace PPSDK = SDK::PaymentProcessor;

namespace Ucs
{
	const QString PluginName = "Ucs";

//--------------------------------------------------------------------------
class UcsBackendPlugin : public SDK::Plugin::IFactory < SDK::PaymentProcessor::Scripting::IBackendScenarioObject >
{
public:
	UcsBackendPlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath):
		mInstancePath(aInstancePath),
		mEnvironment(aFactory)
	{}

public:
	/// Возвращает название плагина.
	virtual QString getPluginName() const { return Ucs::PluginName; }

	/// Возвращает параметры плагина.
	virtual QVariantMap getConfiguration() const { return QVariantMap(); }

	/// Настраивает плагин.
	virtual void setConfiguration(const QVariantMap & aParameters) { Q_UNUSED(aParameters); }

	/// Возвращает имя файла конфигурации без расширения (ключ + идентификатор).
	virtual QString getConfigurationName() const { return mInstancePath; }

	/// Сохраняет конфигурацию плагина в постоянное хранилище (.ini файл или хранилище прикладной программы).
	virtual bool saveConfiguration() { return mEnvironment->saveConfiguration(Ucs::PluginName, getConfiguration()); }

	/// Проверяет успешно ли инициализировался плагин при создании.
	virtual bool isReady() const { return true; }

	/// Возвращает список имен классов, которые создает фабрика.
	virtual QStringList getClassNames() const { return QStringList(Ucs::PluginName); }

	/// Создает класс c заданным именем.
	virtual PPSDK::Scripting::IBackendScenarioObject * create(const QString & aClassName) const
	{
		PPSDK::ICore * core = dynamic_cast<PPSDK::ICore *>(mEnvironment->getInterface(PPSDK::CInterfaces::ICore));
		return Ucs::API::getInstance(core, mEnvironment->getLog(PluginName)).data();
	}

private:
	QString mInstancePath;
	SDK::Plugin::IEnvironment * mEnvironment;
};

}