/* @file База для фабрики платежей. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>

// Project
#include "PaymentFactoryBase.h"

namespace CPaymentFactory
{
	const char LogName[] = "Ad";
}

//------------------------------------------------------------------------------
PaymentFactoryBase::PaymentFactoryBase(SDK::Plugin::IEnvironment * aFactory, const QString & aInstancePath)
	: mInitialized(false),
	  mFactory(aFactory),
	  mInstancePath(aInstancePath),
	  mCore(0),
	  mCryptEngine(0)
{
	try
	{
		mCore = dynamic_cast<SDK::PaymentProcessor::ICore *>(mFactory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
		mCryptEngine = mCore->getCryptService()->getCryptEngine();
		mNetwork = mCore->getNetworkService()->getNetworkTaskManager();

		mInitialized = true;
	}
	catch (const SDK::PaymentProcessor::ServiceIsNotImplemented & e)
	{
		mInitialized = false;

		LOG(getLog(), LogLevel::Error, QString("Failed to initialize payment factory: %1.").arg(e.what()));
	}
}

//------------------------------------------------------------------------------
QVariantMap PaymentFactoryBase::getConfiguration() const
{
	return mParameters;
}

//------------------------------------------------------------------------------
void PaymentFactoryBase::setConfiguration(const QVariantMap & aParameters)
{
	mParameters = aParameters;
}

//------------------------------------------------------------------------------
QString PaymentFactoryBase::getConfigurationName() const
{
	return mInstancePath;
}

//------------------------------------------------------------------------------
bool PaymentFactoryBase::saveConfiguration()
{
	// У плагина нет параметров
	return true;
}

//------------------------------------------------------------------------------
bool PaymentFactoryBase::isReady() const
{
	return true;
}

//------------------------------------------------------------------------------
bool PaymentFactoryBase::restorePayment(SDK::PaymentProcessor::IPayment * aPayment, const QList<SDK::PaymentProcessor::IPayment::SParameter> & aParameters)
{
	return aPayment->restore(aParameters);
}

//------------------------------------------------------------------------------
void PaymentFactoryBase::setSerializer(boost::function<bool(SDK::PaymentProcessor::IPayment *)> aSerializer)
{
	mSerializer = aSerializer;
}

//------------------------------------------------------------------------------
bool PaymentFactoryBase::convertPayment(const QString & /*aTargetType*/, SDK::PaymentProcessor::IPayment * /*aPayment*/)
{
	return false;
}

//------------------------------------------------------------------------------
bool PaymentFactoryBase::savePayment(SDK::PaymentProcessor::IPayment * aPayment)
{
	if (mSerializer)
	{
		return mSerializer(aPayment);
	}

	return false;
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::ICore * PaymentFactoryBase::getCore() const
{
	return mCore;
}

//------------------------------------------------------------------------------
ILog * PaymentFactoryBase::getLog(const char * aLogName /*= nullptr*/) const
{
	return mFactory->getLog(aLogName ? aLogName : CPaymentFactory::LogName);
}

//------------------------------------------------------------------------------
ICryptEngine * PaymentFactoryBase::getCryptEngine() const
{
	return mCryptEngine;
}

//------------------------------------------------------------------------------
NetworkTaskManager * PaymentFactoryBase::getNetworkTaskManager() const
{
	return mNetwork;
}

//------------------------------------------------------------------------------
