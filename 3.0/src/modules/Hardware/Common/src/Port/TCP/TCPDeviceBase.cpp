/* @file Базовый класс устройств на TCP-порту. */

// Modules
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/IOPorts/TCPPort.h"

// Project
#include "TCPDeviceBase.h"

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
template class TCPDeviceBase<PortPollingDeviceBase<ProtoPrinter>>;
template class TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>;

//--------------------------------------------------------------------------------
template <class T>
TCPDeviceBase<T>::TCPDeviceBase()
{
}

//--------------------------------------------------------------------------------
template <class T>
bool TCPDeviceBase<T>::checkConnectionAbility()
{
	if (!checkError(IOPortStatusCode::Error::NotSet, [&] () -> bool { return mIOPort; }, "IO port is not set"))
	{
		return false;
	}

	setConfigParameter(CHardwareSDK::RequiredDevice, QVariant::fromValue(dynamic_cast<IDevice *>(mIOPort)));

	QString address = mIOPort->getDeviceConfiguration()[CHardwareSDK::Port::TCP::IP].toString();

	return checkError(IOPortStatusCode::Error::NotConnected,  [&] () -> bool { return mIOPort->isExist() || mIOPort->deviceConnected(); }, "IO port is not connected") &&
	       checkError(IOPortStatusCode::Error::NotConfigured, [&] () -> bool { return !address.isEmpty(); }, "IO port is not set correctly") &&
	       checkError(IOPortStatusCode::Error::Busy,          [&] () -> bool { return mIOPort->open(); }, "device cannot open port");
}

//--------------------------------------------------------------------------------
template <class T>
bool TCPDeviceBase<T>::checkPort()
{
	if (mIOPort->isExist())
	{
		return true;
	}
	else if (!mIOPort->deviceConnected())
	{
		return false;
	}

	return checkExistence();
}

#define MAKE_TCP_PORT_PARAMETER(aName, aType) TTCPDevicePortParameter aName = mPortParameters[CHardwareSDK::Port::TCP::aType]; \
	if (aName.isEmpty()) { toLog(LogLevel::Error, mDeviceName + QString(": %1 are empty").arg(#aName)); return false; }

//--------------------------------------------------------------------------------
template <class T>
bool TCPDeviceBase<T>::makeSearchingList()
{
	MAKE_TCP_PORT_PARAMETER(IPs, IP);
	MAKE_TCP_PORT_PARAMETER(portNumbers, Number);

	foreach(const QVariant & IP, IPs)
	{
		foreach(const QVariant & portNumber, portNumbers)
		{
			STCPPortParameters params(IP, portNumber);
			mSearchingPortParameters.append(params);
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
IDevice::IDetectingIterator * TCPDeviceBase<T>::getDetectingIterator()
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
bool TCPDeviceBase<T>::find()
{
	QVariantMap portParameters;
	portParameters.insert(CHardwareSDK::Port::TCP::IP, mCurrentParameter.IP);
	portParameters.insert(CHardwareSDK::Port::TCP::Number, mCurrentParameter.number);
	mIOPort->setDeviceConfiguration(portParameters);

	return T::find();
}

//--------------------------------------------------------------------------------
template <class T>
bool TCPDeviceBase<T>::moveNext()
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
