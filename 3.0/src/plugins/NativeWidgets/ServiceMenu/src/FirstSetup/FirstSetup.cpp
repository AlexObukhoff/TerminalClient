/* @file Виджет первоначальной настройки терминала */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWidgets/QGraphicsScene>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>

// Проект
#include "MessageBox/MessageBox.h"
#include "Backend/ServiceMenuBackend.h"
#include "FirstSetup.h"

//--------------------------------------------------------------------------
namespace CFirstSetup
{
	const QString PluginName = "FirstSetup";
}

//--------------------------------------------------------------------------
namespace
{

/// Конструктор плагина.
SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
{
	return new FirstSetup(aFactory, aInstancePath);
}

}

REGISTER_PLUGIN(SDK::Plugin::makePath(SDK::PaymentProcessor::Application, SDK::PaymentProcessor::CComponents::GraphicsItem, CFirstSetup::PluginName), &CreatePlugin);

//--------------------------------------------------------------------------
FirstSetup::FirstSetup(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	: mMainWidget(0),
	  mEnvironment(aFactory),
    mInstancePath(aInstancePath),
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

		mWizardFrame = new WizardFrame(mBackend.data());
		mWizardFrame->initialize();
		mWizardFrame->setStatus(QObject::tr("#cyberplat_copyright"));

		mMainWidget->setWidget(mWizardFrame);
		mMainWidget->setScale(qMin(core->getGUIService()->getScreenSize(0).width() / qreal(mWizardFrame->width()),
			core->getGUIService()->getScreenSize(0).height() / qreal(mWizardFrame->height())));

		qreal newWidgetWidth = core->getGUIService()->getScreenSize(0).width() / mMainWidget->scale();
		mMainWidget->setMinimumWidth(newWidgetWidth);
		mMainWidget->setMaximumWidth(newWidgetWidth);

		qreal newWidgetHeight = core->getGUIService()->getScreenSize(0).height() / mMainWidget->scale();
		mMainWidget->setMinimumHeight(newWidgetHeight);
		mMainWidget->setMaximumHeight(newWidgetHeight);
	}
}

//--------------------------------------------------------------------------
FirstSetup::~FirstSetup()
{
	if (mMainWidget)
	{
		mWizardFrame->shutdown();
		mMainWidget->deleteLater();
	}
}

//--------------------------------------------------------------------------
QString FirstSetup::getPluginName() const
{
	return CFirstSetup::PluginName;
}

//--------------------------------------------------------------------------
QVariantMap FirstSetup::getConfiguration() const
{
	return mParameters;
}

//--------------------------------------------------------------------------
void FirstSetup::setConfiguration(const QVariantMap & aParameters)
{
	mParameters = aParameters;
}

//--------------------------------------------------------------------------
QString FirstSetup::getConfigurationName() const
{
	return mInstancePath;
}

//--------------------------------------------------------------------------
bool FirstSetup::saveConfiguration()
{
	return true;
}

//--------------------------------------------------------------------------
bool FirstSetup::isReady() const
{
	return mIsReady;
}

//---------------------------------------------------------------------------
void FirstSetup::show()
{
}

//---------------------------------------------------------------------------
void FirstSetup::hide()
{
}

//---------------------------------------------------------------------------
void FirstSetup::notify(const QString & /*aReason*/, const QVariantMap & aParameters)
{
	GUI::MessageBox::emitSignal(aParameters);
}

//---------------------------------------------------------------------------
void FirstSetup::reset(const QVariantMap & /*aParameters*/)
{
}

//---------------------------------------------------------------------------
QQuickItem * FirstSetup::getWidget() const
{
	//return mMainWidget;
	//FIXME
	return nullptr;
}

//---------------------------------------------------------------------------
QVariantMap FirstSetup::getContext() const
{
	// TODO
	return QVariantMap();
}

//---------------------------------------------------------------------------
bool FirstSetup::isValid() const
{ 
	return mMainWidget != 0; 
}

//---------------------------------------------------------------------------
QString FirstSetup::getError() const
{
	return QString();
}

//---------------------------------------------------------------------------
