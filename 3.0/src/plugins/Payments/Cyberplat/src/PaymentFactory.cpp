/* @file Фабрика платежей. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMutexLocker>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/CyberPlat/RequestSender.h>

// Project
#include "PinGetCardListRequest.h"
#include "PinGetCardListResponse.h"
#include "Payment.h"
#include "PinPayment.h"
#include "DealerPayment.h"
#include "MultistagePayment.h"
#include "PaymentFactory.h"

//------------------------------------------------------------------------------
namespace CPaymentFactory
{
	const char PluginName[] = "CyberplatPayments";
}

namespace CProcessorType
{
	const QString Cybeplat    = "cyberplat";
	const QString CybeplatPin = "cyberplat_pin";
	const QString Dealer      = "dealer";
	const QString Multistage  = "multistage";
}

//------------------------------------------------------------------------------
namespace
{

/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin * CreatePaymentFactory(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
{
	return new PaymentFactory(aFactory, aInstancePath);
}

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN(SDK::Plugin::makePath(SDK::PaymentProcessor::Application, SDK::PaymentProcessor::CComponents::PaymentFactory, CPaymentFactory::PluginName), &CreatePaymentFactory);
}

//------------------------------------------------------------------------------
PaymentFactory::PaymentFactory(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	: PaymentFactoryBase(aFactory, aInstancePath), 
	  mPinLoader(0)
{
}

//------------------------------------------------------------------------------
QString PaymentFactory::getPluginName() const
{
	return CPaymentFactory::PluginName;
}

//------------------------------------------------------------------------------
bool PaymentFactory::initialize()
{
	mPinLoader = new PinLoader(this);
	return true;
}

//------------------------------------------------------------------------------
void PaymentFactory::shutdown()
{
	delete mPinLoader;
	mPinLoader = 0;
}

//------------------------------------------------------------------------------
QStringList PaymentFactory::getSupportedPaymentTypes() const
{
	return QStringList()
		<< CProcessorType::Cybeplat
		<< CProcessorType::CybeplatPin
		<< CProcessorType::Dealer
		<< CProcessorType::Multistage;
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::IPayment * PaymentFactory::createPayment(const QString & aType)
{
	if (aType.toLower() == CProcessorType::Cybeplat)
	{
		return new Payment(this);
	}
	else if (aType.toLower() == CProcessorType::CybeplatPin)
	{
		return new PinPayment(this);
	}
	else if (aType.toLower() == CProcessorType::Dealer)
	{
		return new DealerPayment(this);
	}
	else if (aType.toLower() == CProcessorType::Multistage)
	{
		return new MultistagePayment(this);
	}

	return nullptr;
}

//------------------------------------------------------------------------------
void PaymentFactory::releasePayment(SDK::PaymentProcessor::IPayment * aPayment)
{
	delete dynamic_cast<Payment *>(aPayment);
}

//------------------------------------------------------------------------------
PPSDK::SProvider PaymentFactory::getProviderSpecification(const PPSDK::SProvider & aProvider)
{
	if (aProvider.processor.type == CProcessorType::CybeplatPin)
	{
		return mPinLoader->getProviderSpecification(aProvider);
	}
	else if (aProvider.processor.type == CProcessorType::Cybeplat)
	{
		return aProvider;
	}
	else if (aProvider.processor.type == CProcessorType::Dealer)
	{
		return aProvider;
	}
	else if (aProvider.processor.type == CProcessorType::Multistage)
	{
		return aProvider;
	}
	else
	{
		return PPSDK::SProvider();
	}
}

//------------------------------------------------------------------------------
QList<SPinCard> PaymentFactory::getPinCardList(qint64 aProvider)
{
	return mPinLoader ? mPinLoader->getPinCardList(aProvider) : QList<SPinCard>();
}

//------------------------------------------------------------------------------
