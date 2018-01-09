/* @file Прокси класс для работы с объектами ядра в скриптах. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCryptographicHash>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/Event.h>

#include <SDK/PaymentProcessor/Scripting/Core.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
Core::Core(ICore * aCore)
	: mCore(aCore),
	  mUserProperties(aCore->getUserProperties()),
	  mPaymentService(aCore),
	  mFundsService(aCore),
	  mPrinterService(aCore),
	  mNetworkService(aCore),
	  mGUIService(aCore),
	  mAdService(aCore),
	  mDeviceService(aCore),
	  mSettings(aCore),
	  mHID(aCore)
{
	ISettingsService * settingsService = aCore->getSettingsService();
	auto terminalSettings = static_cast<SDK::PaymentProcessor::TerminalSettings *>(settingsService->getAdapter(CAdapterNames::TerminalAdapter));

	mPaymentService.setForcePayOffline(terminalSettings->getCommonSettings().skipCheckWhileNetworkError);

	connect(&mUserProperties, SIGNAL(updated()), SIGNAL(userPropertiesUpdated()));
}

//------------------------------------------------------------------------------
void Core::installService(const QString & aName, QObject * aService)
{
	mServices[aName] = aService;
}

//------------------------------------------------------------------------------
void Core::setLog(ILog * aLog)
{
	mLog.setLog(aLog);
	mHID.setLog(aLog);
}

//------------------------------------------------------------------------------
ICore * Core::getCore() const
{
	return mCore;
}

//------------------------------------------------------------------------------
QObject * Core::getService(const QString & aName)
{
	if (mServices.contains(aName))
	{
		return mServices[aName];
	}
	else
	{
		return 0;
	}
}

//------------------------------------------------------------------------------
QObject * Core::getPayment()
{
	return &mPaymentService;
}

//------------------------------------------------------------------------------
QObject * Core::getPrinter()
{
	return &mPrinterService;
}

//------------------------------------------------------------------------------
QObject * Core::getCharge()
{
	return &mFundsService;
}

//------------------------------------------------------------------------------
QObject * Core::getHID()
{
	return &mHID;
}

//------------------------------------------------------------------------------
QObject * Core::getNetwork()
{
	return &mNetworkService;
}

//------------------------------------------------------------------------------
QObject * Core::getGraphics()
{
	return &mGUIService;
}

//------------------------------------------------------------------------------
QObject * Core::getAd()
{
	return &mAdService;
}

//------------------------------------------------------------------------------
QObject * Core::getHardware()
{
	return &mDeviceService;
}

//------------------------------------------------------------------------------
QObject * Core::getSettings()
{
	return &mSettings;
}

//------------------------------------------------------------------------------
QObject * Core::getLog()
{
	return &mLog;
}

//------------------------------------------------------------------------------
QObject * Core::getUserProperties()
{
	return &mUserProperties;
}

//------------------------------------------------------------------------------
void Core::postEvent(int aEvent, QVariant aParameters)
{
	QMetaObject::invokeMethod(this, "onPostEvent", Qt::QueuedConnection,
		Q_ARG(int, aEvent), Q_ARG(QVariant, aParameters));
}

//------------------------------------------------------------------------------
void Core::onPostEvent(int aEvent, QVariant aParameters) const
{
	IEventService * service = mCore->getEventService();
	
	if (service)
	{
		service->sendEvent(Event(aEvent, "ScriptingCore::postEvent", aParameters));
	}
}

//------------------------------------------------------------------------------
QString Core::getMD5Hash(const QString & aSource)
{
	return QCryptographicHash::hash(aSource.toLatin1(), QCryptographicHash::Md5).toHex();
}

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
