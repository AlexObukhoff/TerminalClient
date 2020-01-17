/* @file Плагин сценария для оплаты картами */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QPair>
#include <QtCore/QSettings>
#include <Common/QtHeadersEnd.h>

// PP SDK
#include <SDK/PaymentProcessor/Core/IGUIService.h>
//#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>
#include <SDK/PaymentProcessor/Scripting/PrinterService.h>
#include <SDK/GUI/IGraphicsHost.h>

// Проект
#include "API.h"
#include "UcsBackend.h"

namespace
{
	/// Конструктор плагина.
	SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	{
		return new Ucs::UcsBackendPlugin(aFactory, aInstancePath);
	}
}

REGISTER_PLUGIN(SDK::Plugin::makePath(SDK::PaymentProcessor::Application, PPSDK::CComponents::ScriptFactory,
	Ucs::PluginName), &CreatePlugin);

//---------------------------------------------------------------------------

