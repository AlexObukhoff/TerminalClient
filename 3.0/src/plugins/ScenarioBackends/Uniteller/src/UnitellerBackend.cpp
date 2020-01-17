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
#include "UnitellerBackend.h"

namespace
{
	/// Конструктор плагина.
	SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	{
		return new CUnitellerBackend::UnitellerBackendPlugin(aFactory, aInstancePath);
	}
}

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN(SDK::Plugin::makePath(SDK::PaymentProcessor::Application, PPSDK::CComponents::ScriptFactory,
	CUnitellerBackend::PluginName), &CreatePlugin);


namespace CUnitellerBackend
{
//---------------------------------------------------------------------------
UnitellerBackendPlugin::UnitellerBackendPlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	: mEnvironment(aFactory), 
	mInstancePath(aInstancePath)
{
}

//---------------------------------------------------------------------------
PPSDK::Scripting::IBackendScenarioObject * UnitellerBackendPlugin::create(const QString & aClassName) const
{
	PPSDK::ICore * core = dynamic_cast<PPSDK::ICore *>(mEnvironment->getInterface(PPSDK::CInterfaces::ICore));

	return new UnitellerCore(core, mEnvironment->getLog(Uniteller::LogName), 
		Uniteller::API::getInstance(mEnvironment->getLog(Uniteller::LogName), core));
}

//---------------------------------------------------------------------------
UnitellerCore::UnitellerCore(SDK::PaymentProcessor::ICore * aCore, ILog * aLog, QSharedPointer<Uniteller::API> aAPI) :
	ILogable(aLog),
	mCore(aCore),
	mCountPINNumbers(0)
{
	connect(&mDummyTimer, SIGNAL(timeout()), this, SIGNAL(onTimeout()));
	mDummyTimer.setInterval(1000);
	mDummyTimer.start();

	connect(aAPI.data(), SIGNAL(readyToCard()), this, SIGNAL(onReadyToCard()));
	connect(aAPI.data(), SIGNAL(error(const QString &)), this, SIGNAL(onError(const QString &)));
	connect(aAPI.data(), SIGNAL(deviceEvent(Uniteller::DeviceEvent::Enum, Uniteller::KeyCode::Enum)), SLOT(onDeviceEvent(Uniteller::DeviceEvent::Enum, Uniteller::KeyCode::Enum)));
	connect(aAPI.data(), SIGNAL(print(const QStringList &)), SLOT(onPrintReceipt(const QStringList &)));
	connect(aAPI.data(), SIGNAL(onlineRequired()), SIGNAL(onOnlineRequired()));
	connect(aAPI.data(), SIGNAL(pinRequired()), SIGNAL(onPINRequired()));
	connect(this, SIGNAL(ejected()), aAPI.data(), SLOT(breakSell()));
}

//---------------------------------------------------------------------------
void UnitellerCore::ejectCard()
{
	emit ejected();
}

//---------------------------------------------------------------------------
void UnitellerCore::onDeviceEvent(Uniteller::DeviceEvent::Enum aEvent, Uniteller::KeyCode::Enum aKeyCode)
{
	switch (aEvent)
	{
	case Uniteller::DeviceEvent::KeyPress:
		switch (aKeyCode)
		{
		case Uniteller::KeyCode::Timeout:
			break;

		case Uniteller::KeyCode::Numeric:
			mCountPINNumbers = qMin(4, mCountPINNumbers + 1);
			emit onEnterPin(mCountPINNumbers ? QString("*").repeated(mCountPINNumbers) : "");
			break;

		case Uniteller::KeyCode::Clear:
			mCountPINNumbers = qMax(0, mCountPINNumbers - 1);
			emit onEnterPin(mCountPINNumbers ? QString("*").repeated(mCountPINNumbers) : "");
			break;

		case Uniteller::KeyCode::Cancel:
			mCountPINNumbers = 0;
			emit onError(tr("#canceled_by_user"));
			break;

		case Uniteller::KeyCode::Enter:
			break;
		}
		break;

	case Uniteller::DeviceEvent::CardInserted:
		mCountPINNumbers = 0;
		emit cardInserted();
		break;

	case Uniteller::DeviceEvent::CardCaptured:
		break;

	case Uniteller::DeviceEvent::CardOut:
		emit cardOut();
		break;

	default:
		break;
	}
}

//---------------------------------------------------------------------------
void UnitellerCore::onPrintReceipt(const QStringList & aLines)
{
	QVariantMap parameters;
	parameters["EMV_DATA"] = aLines.join("[br]");

	SDK::PaymentProcessor::Scripting::Core * scriptingCore = static_cast<SDK::PaymentProcessor::Scripting::Core *>
		(dynamic_cast<SDK::GUI::IGraphicsHost *>(mCore->getGUIService())->getInterface<QObject>(SDK::PaymentProcessor::Scripting::CProxyNames::Core));

	SDK::PaymentProcessor::Scripting::PrinterService * ps = static_cast<SDK::PaymentProcessor::Scripting::PrinterService *>(scriptingCore->property("printer").value<QObject *>());

	ps->printReceipt("", parameters, "emv", true);
}

}
//---------------------------------------------------------------------------
