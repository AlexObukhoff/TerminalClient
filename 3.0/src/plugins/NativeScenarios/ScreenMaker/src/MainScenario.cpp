/* @file Плагин сценария для создания скриншотов */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QWidget>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QImage>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IExternalInterface.h>

// PP SDK
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>
#include <SDK/GUI/IGraphicsHost.h>

// Проект
#include "ScenarioPlugin.h"
#include "MainScenario.h"

//--------------------------------------------------------------------------
namespace
{
	/// Конструктор плагина.
	SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	{
		return new ScreenMaker::MainScenarioPlugin(aFactory, aInstancePath);
	}
}

REGISTER_PLUGIN(SDK::Plugin::makePath(SDK::PaymentProcessor::Application, PPSDK::CComponents::ScenarioFactory,
	CScenarioPlugin::PluginName), &CreatePlugin);

namespace ScreenMaker
{

//---------------------------------------------------------------------------
MainScenario::MainScenario(SDK::PaymentProcessor::ICore * aCore, ILog * aLog)
	: Scenario(CScenarioPlugin::PluginName, aLog),
	  mCore(aCore),
		mDrawAreaWindow(nullptr)
{
	QString path = static_cast<SDK::PaymentProcessor::TerminalSettings *>(mCore->getSettingsService()->
		getAdapter(PPSDK::CAdapterNames::TerminalAdapter))->getAppEnvironment().userDataPath + QDir::separator() + "interface.ini";

	int displayIndex = 0;

	QSettings settings(path, QSettings::IniFormat);
	foreach(QString key, settings.allKeys())
	{
		if (key == "interface/display")
		{
			displayIndex = settings.value(key).toInt();
			break;
		}
	}

	QRect rect = mCore->getGUIService()->getScreenSize(displayIndex);

	mDrawAreaWindow = new DrawAreaWindow(mCore, rect);
}

//---------------------------------------------------------------------------
MainScenario::~MainScenario()
{
}

//---------------------------------------------------------------------------
bool MainScenario::initialize(const QList<GUI::SScriptObject> & /*aScriptObjects*/)
{
	return true;
}

//---------------------------------------------------------------------------
void MainScenario::start(const QVariantMap & /*aContext*/)
{
	//mCore->getGUIService()->show("ScreenMakerWidget", QVariantMap());
	mDrawAreaWindow->updateImage(mCore->getGUIService()->getScreenshot().toImage());
	mDrawAreaWindow->setVisible(true);
}

//---------------------------------------------------------------------------
void MainScenario::stop()
{
	mTimeoutTimer.stop();
	mDrawAreaWindow->setVisible(false);
}

//---------------------------------------------------------------------------
void MainScenario::pause()
{
}

//---------------------------------------------------------------------------
void MainScenario::resume(const QVariantMap & /*aContext*/)
{
}

//---------------------------------------------------------------------------
void MainScenario::signalTriggered(const QString & /*aSignal*/, const QVariantMap & /*aArguments*/)
{
	QVariantMap parameters;
	emit finished(parameters);
}

//---------------------------------------------------------------------------
QString MainScenario::getState() const
{
	return QString("main");
}

//---------------------------------------------------------------------------
void MainScenario::onTimeout()
{
	signalTriggered("finish", QVariantMap());
}

//--------------------------------------------------------------------------
bool MainScenario::canStop()
{
	return true;
}

}

//---------------------------------------------------------------------------
