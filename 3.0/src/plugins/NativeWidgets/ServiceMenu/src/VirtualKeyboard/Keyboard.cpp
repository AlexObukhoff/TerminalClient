/* @file Плагин виртуальной клавиатуры */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtWidgets/QWidget>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>

// Project
#include "KeyboardWindow.h"
#include "Keyboard.h"

//--------------------------------------------------------------------------
namespace CKeyboard
{
	const QString PluginName = "VirtualKeyboard";
}

//--------------------------------------------------------------------------
namespace
{

/// Конструктор плагина.
SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
{
	return new Keyboard(aFactory, aInstancePath);
}

}

REGISTER_PLUGIN(SDK::Plugin::makePath(SDK::PaymentProcessor::Application, SDK::PaymentProcessor::CComponents::GraphicsItem, CKeyboard::PluginName), &CreatePlugin);

//--------------------------------------------------------------------------
Keyboard::Keyboard(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	: mMainWidget(0),
	  mEnvironment(aFactory),
	  mInstancePath(aInstancePath),
	  mKeyboardWindow(0),
	  mIsReady(false)
{
	SDK::PaymentProcessor::ICore * core = 
		dynamic_cast<SDK::PaymentProcessor::ICore *>(mEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

	mIsReady = core != 0;

	if (mIsReady)
	{
		mMainWidget = new QGraphicsProxyWidget();

		mKeyboardWindow = new KeyboardWindow();
		mKeyboardWindow->initialize();

		mMainWidget->setWidget(mKeyboardWindow);
		mMainWidget->setFlag(QGraphicsItem::ItemIsFocusable, false);

		mMainWidget->setScale(qMin(1.0, qreal(qMin(core->getGUIService()->getScreenSize(0).width() / qreal(mKeyboardWindow->width()),
			core->getGUIService()->getScreenSize(0).height() / qreal(mKeyboardWindow->height())))));
	}
}

//--------------------------------------------------------------------------
Keyboard::~Keyboard()
{
	if (mMainWidget)
	{
		mKeyboardWindow->shutdown();
		mMainWidget->deleteLater();
	}
}

//--------------------------------------------------------------------------
QString Keyboard::getPluginName() const
{
	return CKeyboard::PluginName;
}

//--------------------------------------------------------------------------
QVariantMap Keyboard::getConfiguration() const
{
	return mParameters;
}

//--------------------------------------------------------------------------
void Keyboard::setConfiguration(const QVariantMap & aParameters)
{
	mParameters = aParameters;
}

//--------------------------------------------------------------------------
QString Keyboard::getConfigurationName() const
{
	return mInstancePath;
}

//--------------------------------------------------------------------------
bool Keyboard::saveConfiguration()
{
	return true;
}

//--------------------------------------------------------------------------
bool Keyboard::isReady() const
{
	return mIsReady;
}

//---------------------------------------------------------------------------
void Keyboard::show()
{
}

//---------------------------------------------------------------------------
void Keyboard::hide()
{
}

//---------------------------------------------------------------------------
void Keyboard::notify(const QString & /*aReason*/, const QVariantMap & /*aParameters*/)
{
}

//---------------------------------------------------------------------------
void Keyboard::reset(const QVariantMap & /*aParameters*/)
{
}

//---------------------------------------------------------------------------
QQuickItem * Keyboard::getWidget() const
{
	//return mMainWidget;
	//FIXME
	return nullptr;
}

//---------------------------------------------------------------------------
QVariantMap Keyboard::getContext() const
{
	// TODO
	return QVariantMap();
}

//---------------------------------------------------------------------------
bool Keyboard::isValid() const
{
	return mMainWidget != 0;
}

//---------------------------------------------------------------------------
QString Keyboard::getError() const
{
	return QString();
}

//---------------------------------------------------------------------------
