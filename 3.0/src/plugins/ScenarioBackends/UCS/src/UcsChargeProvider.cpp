/* @file Реализация плагина. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QPointer>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/Plugins/IPluginEnvironment.h>
#include <SDK/Plugins/IExternalInterface.h>

// SDK
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>

// Проект
#include "UcsChargeProvider.h"

namespace PPSDK = SDK::PaymentProcessor;

namespace CUcsChargeProvider
{
	const QString PluginName = "UcsChargeProvider";

	QPointer<UcsChargeProvider> & plugin()
	{
		static QPointer<UcsChargeProvider> p;

		return p;
	}
}

//------------------------------------------------------------------------------
namespace
{
/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
{
	if (CUcsChargeProvider::plugin().isNull())
	{
		CUcsChargeProvider::plugin() = new UcsChargeProvider(aFactory, aInstancePath);
	}

	return CUcsChargeProvider::plugin();
}
}

static SDK::Plugin::TParameterList EnumerateParameters()
{
	return SDK::Plugin::TParameterList()
		<< SDK::Plugin::SPluginParameter(
			SDK::Plugin::Parameters::Singleton,
			SDK::Plugin::SPluginParameter::Bool,
			false,
			SDK::Plugin::Parameters::Singleton,
			QString(), true, QVariantMap(), true); 
}

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN_WITH_PARAMETERS(SDK::Plugin::makePath(SDK::PaymentProcessor::Application,
	SDK::PaymentProcessor::CComponents::ChargeProvider, CUcsChargeProvider::PluginName), &CreatePlugin, &EnumerateParameters);

//------------------------------------------------------------------------------
UcsChargeProvider::UcsChargeProvider(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath) :
	ILogable(aFactory->getLog(Ucs::LogName)),
	mFactory(aFactory),
	mInstancePath(aInstancePath),
	mCore(dynamic_cast<SDK::PaymentProcessor::ICore *>(aFactory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore))),
	mApi(Ucs::API::getInstance(mCore, aFactory->getLog(Ucs::LogName)))
{
	qRegisterMetaType<SDK::PaymentProcessor::SNote>("SDK::PaymentProcessor::SNote");

	mDealerSettings = dynamic_cast<PPSDK::DealerSettings *>(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::DealerAdapter));

	connect(mApi.data(), SIGNAL(saleComplete(double, int, const QString &, const QString &)),
		SLOT(onSaleComplete(double, int, const QString &, const QString &)));

	connect(mApi.data(), SIGNAL(encashmentComplete()), SLOT(onEncashmentComplete()));

	mCore->getEventService()->subscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event &)));
}

//------------------------------------------------------------------------------
UcsChargeProvider::~UcsChargeProvider()
{
}

//------------------------------------------------------------------------------
QString UcsChargeProvider::getPluginName() const
{
	return CUcsChargeProvider::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap UcsChargeProvider::getConfiguration() const
{
	return mParameters;
}

//------------------------------------------------------------------------------
void UcsChargeProvider::setConfiguration(const QVariantMap & aParameters)
{
	mParameters = aParameters;

	mApi->setupRuntime(aParameters.value(ParamRuntimePath).toString());
}

//------------------------------------------------------------------------------
QString UcsChargeProvider::getConfigurationName() const
{
	return mInstancePath;
}

//------------------------------------------------------------------------------
bool UcsChargeProvider::saveConfiguration()
{
	// У плагина нет параметров
	return true;
}

//------------------------------------------------------------------------------
bool UcsChargeProvider::isReady() const
{
	return true;
}

//------------------------------------------------------------------------------
bool UcsChargeProvider::subscribe(const char * aSignal, QObject * aReceiver, const char * aSlot)
{
	return QObject::connect(this, aSignal, aReceiver, aSlot, Qt::ConnectionType(Qt::UniqueConnection | Qt::QueuedConnection));
}

//------------------------------------------------------------------------------
bool UcsChargeProvider::unsubscribe(const char * aSignal, QObject * aReceiver)
{
	return QObject::disconnect(aSignal, aReceiver);
}

//------------------------------------------------------------------------------
QString UcsChargeProvider::getMethod()
{
	return mApi->isReady() ? "card_ucs" : QString();
}

//------------------------------------------------------------------------------
bool UcsChargeProvider::enable(PPSDK::TPaymentAmount aMaxAmount)
{
	return mApi->enable(aMaxAmount);
}

//------------------------------------------------------------------------------
bool UcsChargeProvider::disable()
{
	// Чистим ресурсы по команде сценария, т.к. нужны дополнительные движения с API
	// mApi->disable();
	return true;
}

//------------------------------------------------------------------------------
void UcsChargeProvider::onEvent(const SDK::PaymentProcessor::Event &  aEvent)
{
	if (aEvent.getType() == PPSDK::EEventType::ProcessEncashment)
	{
		mApi->encashment(false);
	}
}

//------------------------------------------------------------------------------
void UcsChargeProvider::onEncashmentComplete()
{
	// TODO - напечатать все чеки отложенных инкассаций 
	// mCore->getPrinterService()->pri
}

//------------------------------------------------------------------------------
void UcsChargeProvider::onSaleComplete(double aAmount, int aCurrency, const QString & aRRN, const QString & aConfirmationCode)
{
	toLog(LogLevel::Normal, QString("Sale complete: %1 RRN:%2 confirmation:%3.").arg(aAmount, 0, 'f', 2).arg(aRRN).arg(aConfirmationCode));

	mCore->getPaymentService()->updatePaymentField(mCore->getPaymentService()->getActivePayment(), PPSDK::IPayment::SParameter("PAY_TOOL", 2, true, false, true));

	emit stacked(PPSDK::SNote(PPSDK::EAmountType::BankCard, aAmount, aCurrency, aRRN));
}

//------------------------------------------------------------------------------
