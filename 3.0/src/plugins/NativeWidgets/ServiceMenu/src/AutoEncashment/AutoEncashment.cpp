/* @file Плагин автоинскасации терминала */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtGui/QGraphicsScene>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>

// Проект
#include "Backend/ServiceMenuBackend.h"
#include "AutoEncashment.h"

//--------------------------------------------------------------------------
namespace CAutoEncashment
{
	const QString PluginName = "AutoEncashment";
}

//--------------------------------------------------------------------------
namespace
{

/// Конструктор плагина.
SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
{
	return new AutoEncashment(aFactory, aInstancePath);
}

}

REGISTER_PLUGIN(SDK::Plugin::makePath(SDK::PaymentProcessor::Application, SDK::PaymentProcessor::CComponents::GraphicsItem, CAutoEncashment::PluginName), &CreatePlugin);

//--------------------------------------------------------------------------
AutoEncashment::AutoEncashment(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	: mMainWidget(0),
	  mEnvironment(aFactory),
	  mInstancePath(aInstancePath),
	  mAutoEncashmentWindow(0),
		mIsReady(false)
{
	SDK::PaymentProcessor::ICore * core = 
		dynamic_cast<SDK::PaymentProcessor::ICore *>(mEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

	if (core)
	{
		mBackend = QSharedPointer<ServiceMenuBackend>(new ServiceMenuBackend(mEnvironment, mEnvironment->getLog(CServiceMenuBackend::LogName)));
	}
	else
	{
		mEnvironment->getLog("ServiceMenu")->write(LogLevel::Error, "Failed to get ICore");
	}

	mIsReady = core != 0;

	if (mIsReady)
	{
		mMainWidget = new QGraphicsProxyWidget();

		mAutoEncashmentWindow = new AutoEncashmentWindow(mBackend.data());
		mAutoEncashmentWindow->initialize();

		mMainWidget->setWidget(mAutoEncashmentWindow);
		mMainWidget->setScale(qMin(core->getGUIService()->getScreenSize(0).width() / qreal(mAutoEncashmentWindow->width()),
			core->getGUIService()->getScreenSize(0).height() / qreal(mAutoEncashmentWindow->height())));

		qreal newWidgetWidth = core->getGUIService()->getScreenSize(0).width() / mMainWidget->scale();
		mMainWidget->setMinimumWidth(newWidgetWidth);
		mMainWidget->setMaximumWidth(newWidgetWidth);

		qreal newWidgetHeight = core->getGUIService()->getScreenSize(0).height() / mMainWidget->scale();
		mMainWidget->setMinimumHeight(newWidgetHeight);
		mMainWidget->setMaximumHeight(newWidgetHeight);
	}
}

//--------------------------------------------------------------------------
AutoEncashment::~AutoEncashment()
{
	if (mMainWidget)
	{
		mAutoEncashmentWindow->shutdown();
		mMainWidget->deleteLater();
	}
}

//--------------------------------------------------------------------------
QString AutoEncashment::getPluginName() const
{
	return CAutoEncashment::PluginName;
}

//--------------------------------------------------------------------------
QVariantMap AutoEncashment::getConfiguration() const
{
	return mParameters;
}

//--------------------------------------------------------------------------
void AutoEncashment::setConfiguration(const QVariantMap & aParameters)
{
	mParameters = aParameters;
}

//--------------------------------------------------------------------------
QString AutoEncashment::getConfigurationName() const
{
	return mInstancePath;
}

//--------------------------------------------------------------------------
bool AutoEncashment::saveConfiguration()
{
	return true;
}

//--------------------------------------------------------------------------
bool AutoEncashment::isReady() const
{
	return mIsReady;
}

//---------------------------------------------------------------------------
void AutoEncashment::show()
{
}

//---------------------------------------------------------------------------
void AutoEncashment::hide()
{
}

//---------------------------------------------------------------------------
void AutoEncashment::notify(const QString & /*aReason*/, const QVariantMap & /*aParameters*/)
{
}

//---------------------------------------------------------------------------
void AutoEncashment::reset(const QVariantMap & /*aParameters*/)
{
	if (mAutoEncashmentWindow)
	{
		mAutoEncashmentWindow->shutdown();
		mAutoEncashmentWindow->initialize();
	}
}

//---------------------------------------------------------------------------
QGraphicsItem * AutoEncashment::getWidget() const
{
	return mMainWidget;
}

//---------------------------------------------------------------------------
QVariantMap AutoEncashment::getContext() const
{
	// TODO
	return QVariantMap();
}

//---------------------------------------------------------------------------
bool AutoEncashment::isValid() const
{
	return mMainWidget != 0;
}

//---------------------------------------------------------------------------
QString AutoEncashment::getError() const
{
	return QString();
}

//---------------------------------------------------------------------------
