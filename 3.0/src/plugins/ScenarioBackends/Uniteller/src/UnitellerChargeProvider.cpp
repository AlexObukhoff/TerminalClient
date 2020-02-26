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
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>

// Проект
#include "UnitellerChargeProvider.h"

namespace PPSDK = SDK::PaymentProcessor;

namespace CUnitellerChargeProvider
{
	const QString PluginName = "UnitellerChargeProvider";
	const QString RuntimePath = "uniteller_runtime_path";

	QPointer<UnitellerChargeProvider> & plugin()
	{
		static QPointer<UnitellerChargeProvider> p;

		return p;
	}
}

//------------------------------------------------------------------------------
namespace
{
/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
{
	if (CUnitellerChargeProvider::plugin().isNull())
	{
		CUnitellerChargeProvider::plugin() = new UnitellerChargeProvider(aFactory, aInstancePath);
	}

	return CUnitellerChargeProvider::plugin();
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
	SDK::PaymentProcessor::CComponents::ChargeProvider, CUnitellerChargeProvider::PluginName), &CreatePlugin, &EnumerateParameters);

//------------------------------------------------------------------------------
UnitellerChargeProvider::UnitellerChargeProvider(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath) :
	ILogable(aFactory->getLog(Uniteller::LogName)),
	mFactory(aFactory),
	mInstancePath(aInstancePath),
	mCore(dynamic_cast<SDK::PaymentProcessor::ICore *>(aFactory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore))),
	mApi(Uniteller::API::getInstance(aFactory->getLog(Uniteller::LogName), mCore)),
	mMaxAmount(0.0)
{
	qRegisterMetaType<SDK::PaymentProcessor::SNote>("SDK::PaymentProcessor::SNote");

	mDealerSettings = dynamic_cast<PPSDK::DealerSettings *>(mCore->getSettingsService()->getAdapter(PPSDK::CAdapterNames::DealerAdapter));

	connect(mApi.data(), SIGNAL(sellComplete(double, int, const QString &, const QString &)), 
		SLOT(onSellComplete(double, int, const QString &, const QString &)));
}

//------------------------------------------------------------------------------
UnitellerChargeProvider::~UnitellerChargeProvider()
{
}

//------------------------------------------------------------------------------
QString UnitellerChargeProvider::getPluginName() const
{
	return CUnitellerChargeProvider::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap UnitellerChargeProvider::getConfiguration() const
{
	return mParameters;
}

//------------------------------------------------------------------------------
void UnitellerChargeProvider::setConfiguration(const QVariantMap & aParameters)
{
	mParameters = aParameters;

	mApi->setupRuntimePath(aParameters.value(CUnitellerChargeProvider::RuntimePath).toString());
	mApi->enable();	
}

//------------------------------------------------------------------------------
QString UnitellerChargeProvider::getConfigurationName() const
{
	return mInstancePath;
}

//------------------------------------------------------------------------------
bool UnitellerChargeProvider::saveConfiguration()
{
	// У плагина нет параметров
	return true;
}

//------------------------------------------------------------------------------
bool UnitellerChargeProvider::isReady() const
{
	return true;
}

//------------------------------------------------------------------------------
bool UnitellerChargeProvider::subscribe(const char * aSignal, QObject * aReceiver, const char * aSlot)
{
	return QObject::connect(this, aSignal, aReceiver, aSlot, Qt::ConnectionType(Qt::UniqueConnection | Qt::QueuedConnection));
}

//------------------------------------------------------------------------------
bool UnitellerChargeProvider::unsubscribe(const char * aSignal, QObject * aReceiver)
{
	return QObject::disconnect(aSignal, aReceiver);
}

//------------------------------------------------------------------------------
QString UnitellerChargeProvider::getMethod()
{
	return mApi->isReady() ? "card_uniteller" : QString();
}

//------------------------------------------------------------------------------
bool UnitellerChargeProvider::enable(PPSDK::TPaymentAmount aMaxAmount)
{
	if (!mApi->isReady())
	{
		toLog(LogLevel::Error, "enable(): API not ready.");
		return false;
	}
	
	mMaxAmount = aMaxAmount;
	quint64 paymentId = mCore->getPaymentService()->getActivePayment();

	QString initialSession = mCore->getPaymentService()->getPaymentField(paymentId, SDK::PaymentProcessor::CPayment::Parameters::InitialSession)
		.value.toString();

	// Откусываем от номера сессии первые 2 символа, т.к. EFTPOS 3.0 такой большой номер заказа не осилит.
	mApi->sell(aMaxAmount, initialSession.right(18), "");

	return true;
}

//------------------------------------------------------------------------------
bool UnitellerChargeProvider::disable()
{
	if (!mApi->isReady())
	{
		toLog(LogLevel::Error, "disable(): API not ready.");
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
void UnitellerChargeProvider::onSellComplete(double aAmount, int aCurrency, const QString & aRRN, const QString & aConfirmationCode)
{
	toLog(LogLevel::Normal, QString("Sell complete: %1 RRN:%2 confirmation:%3.").arg(aAmount, 0, 'f', 2).arg(aRRN).arg(aConfirmationCode));

	mCore->getPaymentService()->updatePaymentField(mCore->getPaymentService()->getActivePayment(), PPSDK::IPayment::SParameter("PAY_TOOL", 2, true, false, true));

	emit stacked(PPSDK::SNote(PPSDK::EAmountType::BankCard, aAmount, aCurrency, aRRN));
}

//------------------------------------------------------------------------------
