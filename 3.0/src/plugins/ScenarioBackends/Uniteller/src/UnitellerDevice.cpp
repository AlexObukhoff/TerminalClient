/* @file Виртуальный HID для ридера самарских транспортных карт. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QFuture>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/WarningLevel.h>

// Modules
#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Common/BaseStatusDescriptions.h"

// Project
#include "UnitellerDevice.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
namespace Uniteller
{
	/// Имя устройства по умолчанию.
	const char DeviceName[] = "Uniteller";
}

//--------------------------------------------------------------------------------
UnitellerDevice::UnitellerDevice() :
	mCore(nullptr)
{
	toLog(LogLevel::Normal, QString("UnitellerDevice main thread %1.").arg((quint32)QThread::currentThreadId()));
}

//--------------------------------------------------------------------------------
UnitellerDevice::~UnitellerDevice()
{
}

//--------------------------------------------------------------------------------
void UnitellerDevice::updateFirmware(const QByteArray &) {}

//--------------------------------------------------------------------------------
bool UnitellerDevice::canUpdateFirmware() { return false; }

//--------------------------------------------------------------------------------
SDK::Driver::IDevice::IDetectingIterator * UnitellerDevice::getDetectingIterator()
{
	resetDetectingIterator();

	return this;
}

//--------------------------------------------------------------------------------
void UnitellerDevice::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	for (auto it = aConfiguration.begin(); it != aConfiguration.end(); ++it)
	{
		setConfigParameter(it.key(), it.value());
	}
}

//--------------------------------------------------------------------------------
QString UnitellerDevice::getName() const
{
	return Uniteller::DeviceName;
}

//--------------------------------------------------------------------------------
void UnitellerDevice::setLog(ILog * aLog)
{
	if (!ILogable::getLog() && aLog->getName() == Uniteller::LogName)
	{
		ILogable::setLog(aLog);

		QSharedPointer<Uniteller::API> api = Uniteller::API::getInstance(aLog, mCore);
		connect(api.data(), SIGNAL(ready()), this, SLOT(onReady()));
		connect(api.data(), SIGNAL(state(int, const QString &, bool)), SLOT(onState(int, const QString &, bool)));
		connect(api.data(), SIGNAL(deviceEvent(Uniteller::DeviceEvent::Enum, Uniteller::KeyCode::Enum)), SLOT(onDeviceEvent(Uniteller::DeviceEvent::Enum, Uniteller::KeyCode::Enum)));
	}
}

//--------------------------------------------------------------------------------
bool UnitellerDevice::subscribe(const char * aSignal, QObject * aReceiver, const char * aSlot)
{
	return connect(this, aSignal, aReceiver, aSlot, Qt::ConnectionType(Qt::UniqueConnection | Qt::QueuedConnection));
}

//--------------------------------------------------------------------------------
bool UnitellerDevice::unsubscribe(const char * aSignal, QObject * aReceiver)
{
	return disconnect(aSignal, aReceiver);
}

//--------------------------------------------------------------------------------
bool UnitellerDevice::release()
{
	QSharedPointer<Uniteller::API> api = Uniteller::API::getInstance(getLog(), mCore);
	
	api->breakSell();
	api->disable();

	return true;
}

//--------------------------------------------------------------------------------
void UnitellerDevice::initialize()
{
	DeviceStatusCode::CSpecifications specifications;
	sendStatus(EWarningLevel::OK, specifications[DeviceStatusCode::OK::Initialization].translation, EStatus::Interface);
}

//--------------------------------------------------------------------------------
void UnitellerDevice::onReady()
{
	DeviceStatusCode::CSpecifications specifications;
	sendStatus(EWarningLevel::OK, specifications[DeviceStatusCode::OK::OK].translation, EStatus::Actual);
}

//--------------------------------------------------------------------------------
void UnitellerDevice::onState(int aState, const QString & aDeviceName, bool aLast)
{
	mStates.push_back(qMakePair(aState, aDeviceName));

	if (aLast)
	{
		EWarningLevel::Enum state = EWarningLevel::OK;
		QString message;

		foreach (auto status, mStates)
		{
			if (status.second == "Host")
			{
				status.second = "Uniteller server";
			}

			switch (status.first)
			{
			case Uniteller::StatusCode::Error:
				state = EWarningLevel::Error;
				message += status.second + ": Error;";
				break;

			case Uniteller::StatusCode::Timeout:
				state = state < EWarningLevel::Warning ? EWarningLevel::Warning : state;
				message += status.second + ": Timeout;";
				break;

			case Uniteller::StatusCode::Unknown:
				state = EWarningLevel::Error;
				message += status.second + ": Unknown;";
				break;

			case Uniteller::StatusCode::Disabled:
				state = EWarningLevel::Error;
				message += status.second + ": Disabled;";
				break;
			}
		}

		DeviceStatusCode::CSpecifications specifications;
		sendStatus(state, message.isEmpty() ? specifications[DeviceStatusCode::OK::OK].translation : message, EStatus::Actual);
		mStates.clear();
	}
}

//--------------------------------------------------------------------------------
void UnitellerDevice::onDeviceEvent(Uniteller::DeviceEvent::Enum aEvent, Uniteller::KeyCode::Enum aKeyCode)
{
	switch (aEvent)
	{
	case Uniteller::DeviceEvent::KeyPress:
		break;

	case Uniteller::DeviceEvent::CardInserted:
		emit inserted(SDK::Driver::ECardType::MSIC, QVariantMap());
		break;

	case Uniteller::DeviceEvent::CardCaptured:
		break;

	case Uniteller::DeviceEvent::CardOut:
		emit ejected();
		break;

	default:
		break;
	}

}

//--------------------------------------------------------------------------------
void UnitellerDevice::onEjected()
{
}

//--------------------------------------------------------------------------------
void UnitellerDevice::sendStatus(EWarningLevel::Enum aLevel, const QString & aMessage, int aStatus)
{
	QString s = QString("%1%2%3").arg(aLevel).arg(aMessage).arg(aStatus);

	if (s != mLastGeneralizedStatus)
	{
		emit status(aLevel, aMessage, aStatus);
	}

	mLastGeneralizedStatus = s;
}

//--------------------------------------------------------------------------------
bool UnitellerDevice::isDeviceReady() const
{
	QSharedPointer<Uniteller::API> api = Uniteller::API::getInstance(getLog(), mCore);

	return api && api->isReady();
}

//--------------------------------------------------------------------------------
bool UnitellerDevice::find()
{
	if (Uniteller::API::getInstance(getLog(), mCore)->isReady())
	{
		setConfigParameter(CHardwareSDK::ModelName, Uniteller::DeviceName);
		
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------
QVariantMap UnitellerDevice::getDeviceConfiguration() const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration;
}

//--------------------------------------------------------------------------------
void UnitellerDevice::setConfigParameter(const QString & aName, const QVariant & aValue)
{
	QWriteLocker lock(&mConfigurationGuard);

	mConfiguration.insert(aName, aValue);
}

//--------------------------------------------------------------------------------
QVariant UnitellerDevice::getConfigParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration.value(aName);
}

//--------------------------------------------------------------------------------
bool UnitellerDevice::containsConfigParameter(const QString & aName) const
{
	QReadLocker lock(&mConfigurationGuard);

	return mConfiguration.contains(aName);
}

//------------------------------------------------------------------------------
void UnitellerDevice::eject()
{
}

//------------------------------------------------------------------------------
void UnitellerDevice::setCore(SDK::PaymentProcessor::ICore * aCore)
{
	mCore = aCore;
}

//--------------------------------------------------------------------------------
