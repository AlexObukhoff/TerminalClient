/* @file Базовый класс устройств на COM-порту. */

// Modules
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/Watchdogs/ProtoWatchdog.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/HID/ProtoHID.h"
#include "Hardware/IOPorts/IOPortStatusCodes.h"

// Project
#include "SerialDeviceBase.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//-------------------------------------------------------------------------------
template class SerialDeviceBase<PortPollingDeviceBase<ProtoPrinter>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoDispenser>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoWatchdog>>;
template class SerialDeviceBase<PortDeviceBase<DeviceBase<ProtoModem>>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoHID>>;

//--------------------------------------------------------------------------------
template <class T>
SerialDeviceBase<T>::SerialDeviceBase() : mPortStatusChanged(false)
{
	// данные порта
	mPortParameters[EParameters::ByteSize].append(8);
	mPortParameters[EParameters::RTS].append(ERTSControl::Enable);
	mPortParameters[EParameters::DTR].append(EDTRControl::Disable);
}

//--------------------------------------------------------------------------------
template <class T>
QStringList SerialDeviceBase<T>::getOptionalPortSettings()
{
	return QStringList()
		<< COMPortSDK::Parity
		<< COMPortSDK::ByteSize
		<< COMPortSDK::StopBits
		<< COMPortSDK::RTS
		<< COMPortSDK::DTR;
}

//--------------------------------------------------------------------------------
template <class T>
bool SerialDeviceBase<T>::release()
{
	removeConfigParameter(CHardwareSDK::RequiredDevice);

	return T::release();
}

//--------------------------------------------------------------------------------
template <class T>
bool SerialDeviceBase<T>::checkConnectionAbility()
{
	if (!checkError(IOPortStatusCode::Error::NotSet, [&] () -> bool { return mIOPort; }, "IO port is not set"))
	{
		return false;
	}

	setConfigParameter(CHardwareSDK::RequiredDevice, QVariant::fromValue(dynamic_cast<IDevice *>(mIOPort)));

	QString systemName = mIOPort->getDeviceConfiguration()[CHardwareSDK::SystemName].toString();

	return checkError(IOPortStatusCode::Error::NotConnected,  [&] () -> bool { return mIOPort->isExist() || mIOPort->deviceConnected(); }, "IO port is not connected") &&
	       checkError(IOPortStatusCode::Error::NotConfigured, [&] () -> bool { return !systemName.isEmpty(); }, "IO port is not set correctly") &&
	       checkError(IOPortStatusCode::Error::Busy,          [&] () -> bool { return mIOPort->open(); }, "device cannot open port");
}

#define MAKE_SERIAL_PORT_PARAMETER(aParameters, aType) TSerialDevicePortParameter aParameters = mPortParameters[EParameters::aType]; \
	if (aParameters.isEmpty()) { toLog(LogLevel::Error, mDeviceName + QString(": %1 are empty").arg(#aParameters)); return false; } \
	if (!optionalPortSettingsEnable && optionalPortSettings.contains(COMPortSDK::aType)) aParameters = TSerialDevicePortParameter() << aParameters[0];

//--------------------------------------------------------------------------------
template <class T>
bool SerialDeviceBase<T>::makeSearchingList()
{
	QStringList optionalPortSettings = getConfigParameter(CHardwareSDK::OptionalPortSettings).toStringList();
	bool optionalPortSettingsEnable  = getConfigParameter(CHardwareSDK::OptionalPortSettingsEnable).toBool();

	MAKE_SERIAL_PORT_PARAMETER(baudRates, BaudRate);
	MAKE_SERIAL_PORT_PARAMETER(parities, Parity);
	MAKE_SERIAL_PORT_PARAMETER(byteSizes, ByteSize);
	MAKE_SERIAL_PORT_PARAMETER(RTSs, RTS);
	MAKE_SERIAL_PORT_PARAMETER(DTRs, DTR);

	foreach (int baudrate, baudRates)
	{
		foreach (int parity, parities)
		{
			foreach (int RTS, RTSs)
			{
				foreach (int DTR, DTRs)
				{
					foreach (int bytesize, byteSizes)
					{
						SSerialPortParameters params(baudrate, parity, RTS, DTR, bytesize);
						mSearchingPortParameters.append(params);
					}
				}
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool SerialDeviceBase<T>::checkExistence()
{
	if (containsConfigParameter(CHardwareSDK::SearchingType))
	{
		QVariantMap configuration;
		configuration.insert(CHardwareSDK::SearchingType, getConfigParameter(CHardwareSDK::SearchingType));
		mIOPort->setDeviceConfiguration(configuration);
	}

	mIOPort->initialize();

	MutexLocker locker(&mExternalMutex);

	if (!checkConnectionAbility())
	{
		return false;
	}

	setPortLoggingType(mIOMessageLogging);
	setPortDeviceName(mDeviceName);

	TPortParameters portParameters;
	mIOPort->getParameters(portParameters);
	TPortParameters mainPortParameters = portParameters;

	//TODO: убрать, когда будет реализован соответствующий функционал в ПП
	setConfigParameter(CHardwareSDK::OptionalPortSettingsEnable, true);

	if (!getConfigParameter(CHardwareSDK::OptionalPortSettingsEnable).toBool())
	{
		QStringList optionalPortSettings = getOptionalPortSettings();

		auto check = [&](const QString & aConfigParameter, int aPortParameter)
		{
			if (!mPortParameters[aPortParameter].contains(portParameters[aPortParameter]) && optionalPortSettings.contains(aConfigParameter))
			{
				toLog(LogLevel::Normal, QString("Change %1: %2 -> %3")
					.arg(EParameters::EnumToString(aPortParameter))
					.arg(parameterDescription(aPortParameter, portParameters[aPortParameter]))
					.arg(parameterDescription(aPortParameter, mPortParameters[aPortParameter][0])));
				portParameters[aPortParameter] = mPortParameters[aPortParameter][0];
			}
		};

		check(COMPortSDK::BaudRate, EParameters::BaudRate);
		check(COMPortSDK::Parity,   EParameters::Parity);
		check(COMPortSDK::ByteSize, EParameters::ByteSize);
		check(COMPortSDK::RTS,      EParameters::RTS);
		check(COMPortSDK::DTR,      EParameters::DTR);
	}

	if (mainPortParameters != portParameters)
	{
		if (!mIOPort->setParameters(portParameters))
		{
			portParameters = mainPortParameters;
		}
		else if (!T::checkExistence())
		{
			if (mConnected)
			{
				return false;
			}

			portParameters = mainPortParameters;

			if (!mIOPort->setParameters(portParameters))
			{
				return false;
			}
		}
	}

	if (mainPortParameters == portParameters)
	{
		auto portLog = [&](int aParameter) -> QString { return QString(EParameters::EnumToString(aParameter)) + " " + parameterDescription(aParameter, portParameters[aParameter]); };

		if (!portParameters.isEmpty())
		{
			toLog(LogLevel::Normal, QString("Port %1 with %2, %3, %4, %5, %6")
				.arg(mIOPort->getName())
				.arg(portLog(EParameters::BaudRate))
				.arg(portLog(EParameters::Parity))
				.arg(portLog(EParameters::RTS))
				.arg(portLog(EParameters::DTR))
				.arg(portLog(EParameters::ByteSize)));

			QStringList logData;

			for (auto parameters = mPortParameters.begin(); parameters != mPortParameters.end(); ++parameters)
			{
				int parameterKey = parameters.key();
				int portValue = portParameters[parameters.key()];

				if (!parameters->contains(portValue))
				{
					logData << QString("%1 = %2").arg(EParameters::EnumToString(parameterKey)).arg(parameterDescription(parameterKey, portValue));
				}
			}

			if (!logData.isEmpty())
			{
				toLog(LogLevel::Warning, QString("%1: Port parameter(s) are inadvisable: %2").arg(mDeviceName).arg(logData.join(", ")));
				mIOPortStatusCodes.insert(IOPortStatusCode::Warning::MismatchParameters);
			}
			else
			{
				mIOPortStatusCodes.remove(IOPortStatusCode::Warning::MismatchParameters);
			}
		}

		if (!T::checkExistence())
		{
			return false;
		}
	}

	if (mIOPort)
	{
		setPortDeviceName(mDeviceName);
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
IDevice::IDetectingIterator * SerialDeviceBase<T>::getDetectingIterator()
{
	if (!mAutoDetectable)
	{
		return nullptr;
	}

	mSearchingPortParameters.clear();
	makeSearchingList();

	mNextParameterIterator = mSearchingPortParameters.begin();

	return this;
}

//--------------------------------------------------------------------------------
template <class T>
bool SerialDeviceBase<T>::find()
{
	TPortParameters portParameters;
	portParameters.insert(EParameters::BaudRate, mCurrentParameter.baudRate);
	portParameters.insert(EParameters::Parity,   mCurrentParameter.parity);
	portParameters.insert(EParameters::RTS,      mCurrentParameter.RTS);
	portParameters.insert(EParameters::DTR,      mCurrentParameter.DTR);
	portParameters.insert(EParameters::ByteSize, mCurrentParameter.byteSize);

	if (!mIOPort->setParameters(portParameters))
	{
		toLog(LogLevel::Error, mDeviceName + ": Failed to set port parameters, unable to perform current action therefore");
		mIOPortStatusCodes.insert(IOPortStatusCode::Error::Busy);

		return false;
	}

	return T::find();
}

//--------------------------------------------------------------------------------
template <class T>
bool SerialDeviceBase<T>::moveNext()
{
	if (mNextParameterIterator >= mSearchingPortParameters.end())
	{
		return false;
	}

	mCurrentParameter = *mNextParameterIterator;
	mNextParameterIterator++;

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool SerialDeviceBase<T>::processStatus(TStatusCodes & aStatusCodes)
{
	TStatusCodes statusCodes = mIOPortStatusCodes;
	bool result = checkConnectionAbility();
	mPortStatusChanged = statusCodes != mIOPortStatusCodes;

	if (!result)
	{
		return false;
	}

	if (!getStatus(aStatusCodes) || aStatusCodes.contains(DeviceStatusCode::Error::NotAvailable))
	{
		TStatusCodes otherStatusCodes = mIOPortStatusCodes;
		checkConnectionAbility();
		mPortStatusChanged = mPortStatusChanged || (otherStatusCodes != mIOPortStatusCodes);

		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
bool SerialDeviceBase<T>::environmentChanged()
{
	return mPortStatusChanged;
}

//--------------------------------------------------------------------------------
double getFrameSize(const TPortParameters & aPortParameters)
{
	int parity   = aPortParameters[EParameters::Parity];
	int bytesize = aPortParameters[EParameters::ByteSize];
	int stop     = aPortParameters[EParameters::StopBits];

	int parityBits = (parity == EParity::No) ? 0 : 1;
	double stopBits = (stop == EStopBits::One) ? 1 : ((stop == EStopBits::Two) ? 2 : 1.5);

	return 1 + bytesize + parityBits + stopBits;
}

//--------------------------------------------------------------------------------
