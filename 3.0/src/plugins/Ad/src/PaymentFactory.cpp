/* @file Фабрика платежей. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMutexLocker>
#include <QtCore/QFile>
#include <QtCore/QTextCodec>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginInitializer.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/ISettingsAdapter.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/CyberPlat/RequestSender.h>

//Modules
#include "AdBackend/Campaign.h"

// Project
#include "AdPayment.h"
#include "AdRemotePlugin.h"

//------------------------------------------------------------------------------
namespace CPaymentFactory
{
	const char PluginName[] = "AdPayments";
	const char ContentName[] = "banner";
}

namespace CProcessorType
{
	const QString Ad = "ad";
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
	: PaymentFactoryBase(aFactory, aInstancePath)
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
	return true;
}

//------------------------------------------------------------------------------
void PaymentFactory::shutdown()
{
}

//------------------------------------------------------------------------------
QStringList PaymentFactory::getSupportedPaymentTypes() const
{
	return QStringList() << CProcessorType::Ad;
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::IPayment * PaymentFactory::createPayment(const QString & aType)
{
	if (aType.toLower() == CProcessorType::Ad)
	{
		AdPayment * adPayment = new AdPayment(this);

		adPayment->setParameter(SDK::PaymentProcessor::IPayment::SParameter("AD_ID", getAdClientInstance(mFactory)->getAd(CPaymentFactory::ContentName).id, true));

		return adPayment;
	}

	return nullptr;
}

//------------------------------------------------------------------------------
void PaymentFactory::releasePayment(SDK::PaymentProcessor::IPayment * aPayment)
{
	delete dynamic_cast<AdPayment *>(aPayment);
}

//------------------------------------------------------------------------------
PPSDK::SProvider PaymentFactory::getProviderSpecification(const PPSDK::SProvider & aProvider)
{
	if (aProvider.processor.type == CProcessorType::Ad)
	{
		QMutexLocker lock(&mMutex);

		PPSDK::SProvider provider = aProvider;
		
		QFile json(QString("%1/%2.json").arg(getAdClientInstance(mFactory)->getContent(CPaymentFactory::ContentName)).arg(CPaymentFactory::ContentName));
		if (json.open(QIODevice::ReadOnly))
		{
			provider.fields += PPSDK::SProvider::json2Fields(QTextCodec::codecForName("UTF-8")->toUnicode(json.readAll()));
		}
		else
		{
			//TODO
			// Поля не прочитали, провайдера не обновили
		}
		
		return provider;
	}
	else
	{
		return PPSDK::SProvider();
	}
}

//------------------------------------------------------------------------------
