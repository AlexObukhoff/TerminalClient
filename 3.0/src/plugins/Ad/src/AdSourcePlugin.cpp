/* @file Плагин - источник рекламы. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <Common/QtHeadersEnd.h>

// Modules
#include <AdBackend/Client.h>

// SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/PaymentProcessor/Components.h>

// Project
#include "AdSourcePlugin.h"
#include "AdRemotePlugin.h"

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
namespace CAdRemotePlugin
{
	const char PluginName[] = "CyberplatAd";
}

//------------------------------------------------------------------------------
namespace
{
	/// Конструктор экземпляра плагина.
	SDK::Plugin::IPlugin * CreateAdSourcePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	{
		return new AdSourcePlugin(aFactory, aInstancePath);
	}

	/// Регистрация плагина в фабрике.
	REGISTER_PLUGIN(SDK::Plugin::makePath(PPSDK::Application, PPSDK::CComponents::AdSource, CAdRemotePlugin::PluginName), &CreateAdSourcePlugin);
}

//------------------------------------------------------------------------------
AdSourcePlugin::AdSourcePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath) :
	mFactory(aFactory),
	mInstancePath(aInstancePath)
{
	mClient = getAdClientInstance(aFactory);

	mCore = dynamic_cast<SDK::PaymentProcessor::ICore *>(mFactory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
}

//------------------------------------------------------------------------------
AdSourcePlugin::~AdSourcePlugin(void)
{
	mClient.clear();
}


//------------------------------------------------------------------------------
QString AdSourcePlugin::getPluginName() const
{
	return CAdRemotePlugin::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap AdSourcePlugin::getConfiguration() const
{
	return QVariantMap();
}

//------------------------------------------------------------------------------
void AdSourcePlugin::setConfiguration(const QVariantMap & aParameters)
{
	Q_UNUSED(aParameters)
}

//------------------------------------------------------------------------------
QString AdSourcePlugin::getConfigurationName() const
{
	return mInstancePath;
}

//------------------------------------------------------------------------------
bool AdSourcePlugin::saveConfiguration()
{
	// У плагина нет параметров
	return true;
}

//------------------------------------------------------------------------------
bool AdSourcePlugin::isReady() const
{
	return true;
}

//------------------------------------------------------------------------------
QString AdSourcePlugin::getContent(const QString & aType) const
{
	return mClient->getContent(aType);
}

//------------------------------------------------------------------------------
void AdSourcePlugin::addEvent(const QString & aType, const QVariantMap & aParameters)
{
	Q_UNUSED(aParameters)

	//TODO use aParameters!!!
	mClient->addEvent(aType);
}

//------------------------------------------------------------------------------
