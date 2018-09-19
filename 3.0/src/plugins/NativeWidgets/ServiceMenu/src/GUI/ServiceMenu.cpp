/* @file Виджет сервисного меню */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QGraphicsScene>
#include <QtCore/QTime>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>

// Project
#include "Backend/MessageBox.h"
#include "Backend/ServiceMenuBackend.h"
#include "ServiceMenu.h"

//--------------------------------------------------------------------------
namespace CServiceMenu
{
	const QString PluginName = "ServiceMenu";
}

namespace PPSDK = SDK::PaymentProcessor;

//--------------------------------------------------------------------------
namespace
{

/// Конструктор плагина.
SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
{
	return new ServiceMenu(aFactory, aInstancePath);
}

}

typedef QMap<int, bool> map_type;
Q_DECLARE_METATYPE(map_type)

static SDK::Plugin::TParameterList EnumerateParameters()
{
	return SDK::Plugin::TParameterList() 
		<< SDK::Plugin::SPluginParameter(
			"columnVisibility",
			SDK::Plugin::SPluginParameter::MultiSet,
			true,
			"columnVisibility",
			QString(), QVariantList())
			
		<< SDK::Plugin::SPluginParameter(
			SDK::Plugin::Parameters::Singleton,
			SDK::Plugin::SPluginParameter::Bool,
			false,
			SDK::Plugin::Parameters::Singleton,
			QString(), true, QVariantMap(), true);
}

REGISTER_PLUGIN_WITH_PARAMETERS(SDK::Plugin::makePath(SDK::PaymentProcessor::Application, SDK::PaymentProcessor::CComponents::GraphicsItem, CServiceMenu::PluginName), 
	&CreatePlugin, &EnumerateParameters);

//--------------------------------------------------------------------------
ServiceMenu::ServiceMenu(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	: mMainWidget(0),
	  mEnvironment(aFactory),
	  mInstancePath(aInstancePath),
	  mMainServiceWindow(0),
	  mIsReady(true)
{
	SDK::PaymentProcessor::ICore * core =
		dynamic_cast<SDK::PaymentProcessor::ICore *>(mEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

	if (core)
	{
		mBackend = QSharedPointer<ServiceMenuBackend>(new ServiceMenuBackend(mEnvironment, mEnvironment->getLog(CServiceMenuBackend::LogName)));
	}
	else
	{
		mEnvironment->getLog(CServiceMenuBackend::LogName)->write(LogLevel::Error, "Failed to get ICore");
	}

	mMainWidget = new QGraphicsProxyWidget;

	mMainServiceWindow = new MainServiceWindow(mBackend.data());
	mMainServiceWindow->initialize();
	mMainWidget->setWidget(mMainServiceWindow);
	mMainWidget->setScale(qMin(core->getGUIService()->getScreenSize(0).width() / qreal(mMainServiceWindow->width()),
		core->getGUIService()->getScreenSize(0).height() / qreal(mMainServiceWindow->height())));

	qreal newWidgetWidth = core->getGUIService()->getScreenSize(0).width() / mMainWidget->scale();
	mMainWidget->setMinimumWidth(newWidgetWidth);
	mMainWidget->setMaximumWidth(newWidgetWidth);

	qreal newWidgetHeight = core->getGUIService()->getScreenSize(0).height() / mMainWidget->scale();
	mMainWidget->setMinimumHeight(newWidgetHeight);
	mMainWidget->setMaximumHeight(newWidgetHeight);
}

//--------------------------------------------------------------------------
ServiceMenu::~ServiceMenu()
{
	if (mMainWidget)
	{
		saveConfiguration();
	}
}

//--------------------------------------------------------------------------
QString ServiceMenu::getPluginName() const
{
	return CServiceMenu::PluginName;
}

//--------------------------------------------------------------------------
QVariantMap ServiceMenu::getConfiguration() const
{
	return mBackend->getConfiguration();
}

//--------------------------------------------------------------------------
void ServiceMenu::setConfiguration(const QVariantMap & aParameters)
{
	mBackend->setConfiguration(aParameters);
}

//--------------------------------------------------------------------------
QString ServiceMenu::getConfigurationName() const
{
	return mInstancePath;
}

//--------------------------------------------------------------------------
bool ServiceMenu::saveConfiguration()
{
	return mEnvironment->saveConfiguration(mInstancePath, mBackend->getConfiguration());
}

//--------------------------------------------------------------------------
bool ServiceMenu::isReady() const
{
	return mIsReady;
}

//---------------------------------------------------------------------------
void ServiceMenu::show()
{
	mBackend->startHeartbeat();
}

//---------------------------------------------------------------------------
void ServiceMenu::hide()
{
	mBackend->stopHeartbeat();
}

//---------------------------------------------------------------------------
void ServiceMenu::notify(const QString & aReason, const QVariantMap & aParameters)
{
	if (aReason.toInt() == PPSDK::EEventType::TryStopScenario)
	{
		mMainServiceWindow->closeServiceMenu(true, QObject::tr("#need_restart_application"), true);
	}
	else
	{
		MessageBox::emitSignal(aParameters);
	}
}

//---------------------------------------------------------------------------
void ServiceMenu::reset(const QVariantMap & /*aParameters*/)
{
	if (mMainServiceWindow)
	{
		mMainServiceWindow->shutdown();
		mMainServiceWindow->initialize();
	}
}

//---------------------------------------------------------------------------
QGraphicsItem * ServiceMenu::getWidget() const
{
	return mMainWidget;
}

//---------------------------------------------------------------------------
QVariantMap ServiceMenu::getContext() const
{
	// TODO
	return QVariantMap();
}

//---------------------------------------------------------------------------
bool ServiceMenu::isValid() const
{
	return mMainWidget != 0;
}

//---------------------------------------------------------------------------
QString ServiceMenu::getError() const
{
	return QString();
}

//---------------------------------------------------------------------------
