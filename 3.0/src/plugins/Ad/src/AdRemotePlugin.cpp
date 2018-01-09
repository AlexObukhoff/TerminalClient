/* @file Клиент обновления рекламы. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>
#include <Common/QtHeadersEnd.h>

// Modules
#include <AdBackend/Client.h>
#include <AdBackend/DatabaseUtils.h>
#include <AdBackend/Campaign.h>

// SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/IRemoteService.h>

// Project
#include "AdRemotePlugin.h"

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
namespace CAdRemotePlugin
{
	const char PluginName[] = "AdRemote";
}

//------------------------------------------------------------------------------
namespace
{
	/// Конструктор экземпляра плагина.
	SDK::Plugin::IPlugin * CreateAdSourcePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	{
		return new AdRemotePlugin(aFactory, aInstancePath);
	}

	/// Регистрация плагина в фабрике.
	REGISTER_PLUGIN(SDK::Plugin::makePath(PPSDK::Application, PPSDK::CComponents::RemoteClient, CAdRemotePlugin::PluginName), &CreateAdSourcePlugin);
}

//------------------------------------------------------------------------
QSharedPointer<Ad::Client> getAdClientInstance(SDK::Plugin::IEnvironment * aFactory)
{
	static QSharedPointer<Ad::Client> client;

	if (client.isNull())
	{
		PPSDK::ICore * core = dynamic_cast<PPSDK::ICore *>(aFactory->getInterface(PPSDK::CInterfaces::ICore));

		client = QSharedPointer<Ad::Client>(new Ad::Client(core, aFactory->getLog(Ad::CClient::LogName), 0));
	}

	return client;
}

//------------------------------------------------------------------------------
AdRemotePlugin::AdRemotePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath) :
	ILogable(aFactory->getLog(Ad::CClient::LogName)),
	mFactory(aFactory),
	mInstancePath(aInstancePath)
{
	mClient = getAdClientInstance(aFactory);

	mCore = dynamic_cast<SDK::PaymentProcessor::ICore *>(mFactory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

	connect(mClient.data(), SIGNAL(contentUpdated()), this, SLOT(needRestart()));
	connect(mClient.data(), SIGNAL(contentExpired()), this, SLOT(needRestart()));
}

//------------------------------------------------------------------------------
AdRemotePlugin::~AdRemotePlugin()
{
	disable();
}

//------------------------------------------------------------------------------
QString AdRemotePlugin::getPluginName() const
{
	return CAdRemotePlugin::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap AdRemotePlugin::getConfiguration() const
{
	return QVariantMap();
}

//------------------------------------------------------------------------------
void AdRemotePlugin::setConfiguration(const QVariantMap & aParameters)
{
	Q_UNUSED(aParameters)
}

//------------------------------------------------------------------------------
QString AdRemotePlugin::getConfigurationName() const
{
	return mInstancePath;
}

//------------------------------------------------------------------------------
bool AdRemotePlugin::saveConfiguration()
{
	// У плагина нет параметров
	return true;
}

//------------------------------------------------------------------------------
bool AdRemotePlugin::isReady() const
{
	return true;
}

//------------------------------------------------------------------------------
void AdRemotePlugin::enable()
{
	toLog(LogLevel::Normal, "Ad updater plugin enabled.");

	QMetaObject::invokeMethod(mClient.data(), "reinitialize",  Qt::QueuedConnection);
}

//------------------------------------------------------------------------------
void AdRemotePlugin::disable()
{
	mClient.clear();

	toLog(LogLevel::Normal, "Ad updater plugin disabled.");
}

//------------------------------------------------------------------------------
void AdRemotePlugin::needRestart()
{
	toLog(LogLevel::Normal, "Send restart command.");

	mCore->getRemoteService()->registerRestartCommand();
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::ICore * AdRemotePlugin::getCore() const
{
	return mCore;
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::IRemoteClient::Capabilities AdRemotePlugin::getCapabilities() const
{
	return SDK::PaymentProcessor::IRemoteClient::UpdateContent;
}

//------------------------------------------------------------------------------
bool AdRemotePlugin::useCapability(ECapability aCapabilty)
{
	if (aCapabilty == IRemoteClient::UpdateContent)
	{
		QMetaObject::invokeMethod(mClient.data(), "update", Qt::QueuedConnection);

		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
