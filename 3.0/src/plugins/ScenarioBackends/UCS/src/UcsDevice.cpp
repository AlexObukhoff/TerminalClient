/* @file Виртуальный HID для ридера самарских транспортных карт. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFuture>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/WarningLevel.h>

// Modules
#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Common/BaseStatusDescriptions.h"

// Project
#include "UcsDevice.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
namespace Ucs
{
	/// Имя устройства по умолчанию.
	const char DeviceName[] = "UCS";
}

//--------------------------------------------------------------------------------
UcsDevice::UcsDevice() :
	mCore(nullptr)
{
}

//--------------------------------------------------------------------------------
UcsDevice::~UcsDevice()
{
}

//--------------------------------------------------------------------------------
void UcsDevice::updateFirmware(const QByteArray &) {}

//--------------------------------------------------------------------------------
bool UcsDevice::canUpdateFirmware() { return false; }

//--------------------------------------------------------------------------------
SDK::Driver::IDevice::IDetectingIterator * UcsDevice::getDetectingIterator()
{
	resetDetectingIterator();

	return this;
}

//--------------------------------------------------------------------------------
void UcsDevice::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	for (auto it = aConfiguration.begin(); it != aConfiguration.end(); ++it)
	{
		setConfigParameter(it.key(), it.value());
	}
}

//--------------------------------------------------------------------------------
QString UcsDevice::getName() const
{
	return Ucs::DeviceName;
}

//--------------------------------------------------------------------------------
void UcsDevice::setLog(ILog * aLog)
{
	if (!ILogable::getLog() && aLog->getName() == Ucs::LogName)
	{
		ILogable::setLog(aLog);
	}
}

//--------------------------------------------------------------------------------
bool UcsDevice::subscribe(const char * aSignal, QObject * aReceiver, const char * aSlot)
{
	return connect(this, aSignal, aReceiver, aSlot, Qt::ConnectionType(Qt::UniqueConnection | Qt::QueuedConnection));
}

//--------------------------------------------------------------------------------
bool UcsDevice::unsubscribe(const char * aSignal, QObject * aReceiver)
{
	return disconnect(aSignal, aReceiver);
}

//--------------------------------------------------------------------------------
bool UcsDevice::release()
{
	Ucs::API::getInstance(mCore, getLog())->disable();
	return true;
}

//--------------------------------------------------------------------------------
void UcsDevice::initialize()
{
	QSharedPointer<Ucs::API> api = Ucs::API::getInstance(mCore, getLog());
	connect(api.data(), SIGNAL(ready()), this, SLOT(onReady()));
	
	DeviceStatusCode::CSpecifications specifications;
	if (api->isReady())
	{
		sendStatus(EWarningLevel::OK, specifications[DeviceStatusCode::OK::OK].translation, EStatus::Actual);
	}
	else
	{
		sendStatus(EWarningLevel::Error, specifications[DeviceStatusCode::Error::ThirdPartyDriverFail].translation, EStatus::Actual);
	}	
}

//--------------------------------------------------------------------------------
void UcsDevice::onReady()
{
	DeviceStatusCode::CSpecifications specifications;
	sendStatus(EWarningLevel::OK, specifications[DeviceStatusCode::OK::OK].translation, EStatus::Actual);
}

//--------------------------------------------------------------------------------
void UcsDevice::onState(int aState, const QString & aDeviceName, bool aLast)
{
	if (aLast)
	{
		EWarningLevel::Enum state = EWarningLevel::OK;
		QString message;

		DeviceStatusCode::CSpecifications specifications;
		sendStatus(state, message.isEmpty() ? specifications[DeviceStatusCode::OK::OK].translation : message, EStatus::Actual);
	}
}

//--------------------------------------------------------------------------------
void UcsDevice::onEjected()
{
}

//--------------------------------------------------------------------------------
void UcsDevice::sendStatus(EWarningLevel::Enum aLevel, const QString & aMessage, int aStatus)
{
	QString s = QString("%1%2%3").arg(aLevel).arg(aMessage).arg(aStatus);

	if (s != mLastGeneralizedStatus)
	{
		emit status(aLevel, aMessage, aStatus);
	}

	mLastGeneralizedStatus = s;
}

//--------------------------------------------------------------------------------
bool UcsDevice::isDeviceReady() const
{
	QSharedPointer<Ucs::API> api = Ucs::API::getInstance(mCore, getLog());

	return api && api->isReady();
}

//--------------------------------------------------------------------------------
bool UcsDevice::find()
{
	if (Ucs::API::getInstance(mCore, getLog())->isReady())
	{
		setConfigParameter(CHardwareSDK::ModelName, Ucs::DeviceName);
		
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------
QVariantMap UcsDevice::getDeviceConfiguration() const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration;
}

//--------------------------------------------------------------------------------
void UcsDevice::setConfigParameter(const QString & aName, const QVariant & aValue)
{
	QWriteLocker lock(&mConfigurationGuard);

	mConfiguration.insert(aName, aValue);
}

//--------------------------------------------------------------------------------
QVariant UcsDevice::getConfigParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration.value(aName);
}

//--------------------------------------------------------------------------------
bool UcsDevice::containsConfigParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration.contains(aName);
}

//------------------------------------------------------------------------------
void UcsDevice::eject()
{
}

//------------------------------------------------------------------------------
void UcsDevice::setCore(SDK::PaymentProcessor::ICore * aCore)
{
	mCore = aCore;
}

//--------------------------------------------------------------------------------
