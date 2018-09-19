/* @file Реализация плагина. */

// SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IPluginFactory.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/GUI/IGraphicsEngine.h>
#include <SDK/PaymentProcessor/Components.h>

// Проект
#include "NativeBackend.h"

//------------------------------------------------------------------------------
namespace CNativeBackend
{
	/// Тип данного графического движка.
	const char Type[] = "native";
	const char PluginName[] = "Native";
	const char Item[] = "item";
}

//------------------------------------------------------------------------------
namespace
{

/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
{
	return new NativeBackend(aFactory, aInstancePath);
}

QVector<SDK::Plugin::SPluginParameter> EnumParameters()
{
	return QVector<SDK::Plugin::SPluginParameter>(1)
		<< SDK::Plugin::SPluginParameter(SDK::Plugin::Parameters::Debug, SDK::Plugin::SPluginParameter::Bool, false,
			QT_TRANSLATE_NOOP("NativeBackendParameters", "#debug_mode"), QT_TRANSLATE_NOOP("NativeBackendParameters", "#debug_mode_howto"),
			false);
}

}

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN_WITH_PARAMETERS(SDK::Plugin::makePath(SDK::PaymentProcessor::Application, SDK::PaymentProcessor::CComponents::GraphicsBackend, CNativeBackend::PluginName), &CreatePlugin, &EnumParameters);

//------------------------------------------------------------------------------
NativeBackend::NativeBackend(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	: mFactory(aFactory),
	  mInstancePath(aInstancePath),
	  mEngine(0),
	  mCore(0)
{
}

//------------------------------------------------------------------------------
NativeBackend::~NativeBackend()
{
}

//------------------------------------------------------------------------------
QString NativeBackend::getPluginName() const
{
	return CNativeBackend::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap NativeBackend::getConfiguration() const
{
	return mParameters;
}

//------------------------------------------------------------------------------
void NativeBackend::setConfiguration(const QVariantMap & aParameters)
{
	mParameters = aParameters;
}

//------------------------------------------------------------------------------
QString NativeBackend::getConfigurationName() const
{
	return mInstancePath;
}

//------------------------------------------------------------------------------
bool NativeBackend::saveConfiguration()
{
	// У плагина нет параметров
	return true;
}

//------------------------------------------------------------------------------
bool NativeBackend::isReady() const
{
	return true;
}

//------------------------------------------------------------------------------
std::weak_ptr<SDK::GUI::IGraphicsItem> NativeBackend::getItem(const SDK::GUI::GraphicsItemInfo & aInfo)
{
	// TODO Context
	TGraphicItemsCache::iterator it = mCachedItems.find(aInfo.name);

	if (it == mCachedItems.end())
	{
		auto plugin = mFactory->getPluginLoader()->createPluginPtr(aInfo.directory + aInfo.name);

		if (!plugin.expired()) 
		{
			mLoadedPlugins.push_back(plugin);

			auto item = std::dynamic_pointer_cast<SDK::GUI::IGraphicsItem, SDK::Plugin::IPlugin>(plugin.lock());
			it = mCachedItems.insert(aInfo.name, item);
		}
		else
		{
			mEngine->getLog()->write(LogLevel::Error, QString("Failed to create '%1' graphics item.").arg(aInfo.name));
		}
	}

	return it == mCachedItems.end() ? std::weak_ptr<SDK::GUI::IGraphicsItem>() : it.value();
}

//------------------------------------------------------------------------------
bool NativeBackend::removeItem(const SDK::GUI::GraphicsItemInfo & aInfo)
{
	foreach(auto item, mCachedItems.values(aInfo.name))
	{
		if (item->getContext() == aInfo.context)
		{
			return mCachedItems.remove(aInfo.name, item) != 0;
		}
	}

	return false;
}

//------------------------------------------------------------------------------
QString NativeBackend::getType() const
{
	return CNativeBackend::Type;
}

//------------------------------------------------------------------------------
QList<SDK::GUI::GraphicsItemInfo> NativeBackend::getItemList()
{
	if (mItemList.isEmpty())
	{
		QStringList items = mFactory->getPluginLoader()->getPluginList(QRegExp("PaymentProcessor\\.GraphicsItem\\..*"));
		
		foreach (const QString & item, items)
		{
			SDK::GUI::GraphicsItemInfo itemInfo;
			itemInfo.name = item.split(".").last(); //Последняя секция - имя айтема
			itemInfo.type = CNativeBackend::Type;
			itemInfo.directory = "PaymentProcessor.GraphicsItem.";
			
			mItemList[itemInfo.name] = itemInfo;
		}
	}
	
	return mItemList.values();
}

//------------------------------------------------------------------------------
bool NativeBackend::initialize(SDK::GUI::IGraphicsEngine * aEngine)
{
	mEngine = aEngine;
	mCore = mEngine->getGraphicsHost()->getInterface<SDK::PaymentProcessor::ICore>(SDK::PaymentProcessor::CInterfaces::ICore);

	return true;
}

//------------------------------------------------------------------------------
void NativeBackend::shutdown()
{
	mCachedItems.clear();

	while (!mLoadedPlugins.empty())
	{
		auto ptr = mLoadedPlugins.front();

		mFactory->getPluginLoader()->destroyPluginPtr(ptr);

		mLoadedPlugins.pop_front();
	}
}

//------------------------------------------------------------------------------