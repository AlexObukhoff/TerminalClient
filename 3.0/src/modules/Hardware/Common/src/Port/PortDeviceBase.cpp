/* @file Базовый класс устройств на порту. */

// Modules
#include "Hardware/Common/PollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/Watchdogs/ProtoWatchdog.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/CardReaders/ProtoMifareReader.h"
#include "Hardware/HID/ProtoHID.h"

// Project
#include "PortDeviceBase.h"

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
template class PortDeviceBase<PollingDeviceBase<ProtoPrinter>>;
template class PortDeviceBase<PollingDeviceBase<ProtoDispenser>>;
template class PortDeviceBase<PollingDeviceBase<ProtoCashAcceptor>>;
template class PortDeviceBase<PollingDeviceBase<ProtoWatchdog>>;
template class PortDeviceBase<DeviceBase<ProtoModem>>;
template class PortDeviceBase<PollingDeviceBase<ProtoFR>>;
template class PortDeviceBase<PollingDeviceBase<ProtoMifareReader>>;
template class PortDeviceBase<PollingDeviceBase<ProtoHID>>;

//--------------------------------------------------------------------------------
template <class T>
PortDeviceBase<T>::PortDeviceBase() : mIOPort(nullptr), mIOMessageLogging(ELoggingType::None), mControlRemoving(false)
{
	mInitializeRepeatCount = 2;
}

//--------------------------------------------------------------------------------
template <class T>
void PortDeviceBase<T>::addPortData()
{
	EPortTypes::Enum portType = mIOPort->getType();
	QString port;

	if (portType == EPortTypes::COM) port = "COM";
	if (portType == EPortTypes::USB) port = "USB";
	if (portType == EPortTypes::TCP) port = "TCP";
	if (portType == EPortTypes::VirtualCOM)  port = "Virtual COM";
	if (portType == EPortTypes::COMEmulator) port = "COM Emulator";

	setDeviceParameter(CDeviceData::Port, port);
}

//--------------------------------------------------------------------------------
template <class T>
void PortDeviceBase<T>::finaliseInitialization()
{
	addPortData();

	mControlRemoving = mIOPort->getDeviceConfiguration()[CHardware::Port::COM::ControlRemoving].toBool();

	T::finaliseInitialization();
}

//--------------------------------------------------------------------------------
template <class T>
bool PortDeviceBase<T>::release()
{
	if (!T::release())
	{
		return false;
	}

	if (mIOPort && !mIOPort->close())
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to close device port");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void PortDeviceBase<T>::setDeviceConfiguration(const QVariantMap & aConfiguration)
{
	T::setDeviceConfiguration(aConfiguration);

	if (aConfiguration.contains(CHardwareSDK::RequiredDevice))
	{
		mIOPort = dynamic_cast<IIOPort *>(aConfiguration[CHardwareSDK::RequiredDevice].value<IDevice *>());
	}

	if (mIOPort)
	{
		QVariantMap configuration;
		TVoidMethod forwardingTask = isAutoDetecting() ? TVoidMethod() : std::bind(&DeviceBase::initialize, this);
		configuration.insert(CHardware::Port::OpeningContext, QVariant::fromValue(forwardingTask));

		mIOPort->setDeviceConfiguration(configuration);
	}
}

//--------------------------------------------------------------------------------
template <class T>
TResult PortDeviceBase<T>::processCommand(char aCommand, QByteArray * aAnswer = nullptr)
{
	return processCommand(aCommand, QByteArray(), aAnswer);
}

//--------------------------------------------------------------------------------
template <class T>
TResult PortDeviceBase<T>::processCommand(const QByteArray & aCommand, QByteArray * aAnswer = nullptr)
{
	return processCommand(aCommand, QByteArray(), aAnswer);
}

//--------------------------------------------------------------------------------
template <class T>
TResult PortDeviceBase<T>::processCommand(char aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr)
{
	return processCommand(QByteArray(1, aCommand), aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
template <class T>
TResult PortDeviceBase<T>::processCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer)
{
	TResult result = execCommand(aCommand, aCommandData, aAnswer);

	if (!mControlRemoving || ((result != CommandResult::NoAnswer) && (result != CommandResult::Transport)))
	{
		return result;
	}

	mIOPort->close();

	if (!mIOPort->open())
	{
		return CommandResult::Port;
	}

	return execCommand(aCommand, aCommandData, aAnswer);
}

//--------------------------------------------------------------------------------
template <class T>
bool PortDeviceBase<T>::checkError(int aError, TBoolMethod aChecking, const QString & aErrorLog)
{
	bool contains = mIOPortStatusCodes.contains(aError);
	mIOPortStatusCodes.remove(aError);

	if (aChecking())
	{
		return true;
	}

	mIOPortStatusCodes.insert(aError);

	if (!contains)
	{
		toLog(LogLevel::Error, mDeviceName + ": " + aErrorLog);
	}

	return false;
};

//--------------------------------------------------------------------------------
template <class T>
bool PortDeviceBase<T>::checkExistence()
{
	MutexLocker locker(&mExternalMutex);

	if (!checkConnectionAbility())
	{
		return false;
	}

	//TODO: сделать настройку плагинов - расширенное логгирование
	QVariantMap configuration;
	configuration.insert(CHardware::Port::IOLogging, QVariant().fromValue(mIOMessageLogging));
	configuration.insert(CHardware::Port::DeviceModelName, mDeviceName);

	mIOPort->setDeviceConfiguration(configuration);

	if (!T::checkExistence())
	{
		return false;
	}

	configuration.insert(CHardware::Port::DeviceModelName, mDeviceName);
	mIOPort->setDeviceConfiguration(configuration);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool PortDeviceBase<T>::processStatus(TStatusCodes & aStatusCodes)
{
	if (!checkPort())
	{
		if (mIOPortStatusCodes.isEmpty())
		{
			checkError(IOPortStatusCode::Error::Busy, [&] () -> bool { return mIOPort->open(); }, "device cannot open port after getting status");
		}

		return false;
	}

	if (getStatus(aStatusCodes) && !aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable))
	{
		return true;
	}

	if (!checkPort())
	{
		if (mIOPortStatusCodes.isEmpty())
		{
			checkError(IOPortStatusCode::Error::Busy, [&] () -> bool { return mIOPort->open(); }, "device cannot open port after getting status");
		}

		return false;
	}

	aStatusCodes.insert(DeviceStatusCode::Error::Unknown);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
SStatusCodeSpecification PortDeviceBase<T>::getStatusCodeSpecification(int aStatusCode) const
{
	return mIOPortStatusCodes.contains(aStatusCode) ? mIOPortStatusCodesSpecification[aStatusCode] : T::getStatusCodeSpecification(aStatusCode);
}

//--------------------------------------------------------------------------------
template <class T>
QString PortDeviceBase<T>::getTrOfNewProcessed(const TStatusCollection & aStatusCollection, EWarningLevel::Enum aWarningLevel)
{
	TStatusCodes statusCodes = aStatusCollection[aWarningLevel];

	foreach(int statusCode, mIOPortStatusCodes)
	{
		if (mIOPortStatusCodesSpecification[statusCode].warningLevel == aWarningLevel)
		{
			statusCodes.insert(statusCode);
		}
	}

	return getStatusTranslations(statusCodes, false);
}

//--------------------------------------------------------------------------------
template <class T>
void PortDeviceBase<T>::emitStatusCodes(TStatusCollection & aStatusCollection, int aExtendedStatus)
{
	if (!mIOPortStatusCodes.isEmpty() && aStatusCollection.contains(EWarningLevel::OK))
	{
		aStatusCollection[EWarningLevel::OK].remove(DeviceStatusCode::OK::OK);
	}

	foreach (int portStatusCode, mIOPortStatusCodes)
	{
		EWarningLevel::Enum warningLevel = mIOPortStatusCodesSpecification[portStatusCode].warningLevel;
		aStatusCollection[warningLevel].insert(portStatusCode);
	}

	T::emitStatusCodes(aStatusCollection, aExtendedStatus);
}

//---------------------------------------------------------------------------
template <class T>
bool PortDeviceBase<T>::canApplyStatusBuffer()
{
	bool portError = std::find_if(mIOPortStatusCodes.begin(), mIOPortStatusCodes.end(), [&] (int aStatusCode) -> bool
		{ return mIOPortStatusCodesSpecification[aStatusCode].warningLevel == EWarningLevel::Error; }) != mIOPortStatusCodes.end();

	return !portError && T::canApplyStatusBuffer();
}

//--------------------------------------------------------------------------------
template <class T>
void PortDeviceBase<T>::doPoll(TStatusCodes & aStatusCodes)
{
	{
		MutexLocker locker(&mExternalMutex);

		if (mLogDate.day() != QDate::currentDate().day())
		{
			mIOPort->initialize();
		}
	}

	T::doPoll(aStatusCodes);
}

//--------------------------------------------------------------------------------
template <class T>
EWarningLevel::Enum PortDeviceBase<T>::getWarningLevel(const TStatusCollection & aStatusCollection)
{
	TStatusCollection portStatusCollection = getStatusCollection(mIOPortStatusCodes, &mIOPortStatusCodesSpecification);
	EWarningLevel::Enum   portWarningLevel = DeviceBase::getWarningLevel(portStatusCollection);
	EWarningLevel::Enum deviceWarningLevel = DeviceBase::getWarningLevel(aStatusCollection);

	return qMax(deviceWarningLevel, portWarningLevel);
}

//--------------------------------------------------------------------------------
template <class T>
void PortDeviceBase<T>::setLog(ILog * aLog)
{
	T::setLog(aLog);

	if (mIOPort)
	{
		mIOPort->setLog(aLog);
	}
}

//--------------------------------------------------------------------------------
