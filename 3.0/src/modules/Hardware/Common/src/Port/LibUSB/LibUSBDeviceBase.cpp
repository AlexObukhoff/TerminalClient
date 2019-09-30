/* @file Базовый класс устройств на LibUSB-порту. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QWriteLocker>
#include <QtCore/QReadLocker>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/IOPorts/LibUSBUtils.h"

// Project
#include "LibUSBDeviceBase.h"

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
#define INSTANCE_LIB_USB_DEVICE(aClass) template class aClass; aClass::TUsageData aClass::mUsageData; QMutex aClass::mUsageDataGuard(QMutex::Recursive);

INSTANCE_LIB_USB_DEVICE(LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>)

//--------------------------------------------------------------------------------
template <class T>
LibUSBDeviceBase<T>::LibUSBDeviceBase()
{
	mIOPort = &mLibUSBPort;

	mDetectingData = CUSBDevice::PDetectingData(new CUSBDevice::DetectingData());
	mReplaceableStatuses << DeviceStatusCode::Error::PowerSupply;
}

//--------------------------------------------------------------------------------
template <class T>
LibUSBDeviceBase<T>::~LibUSBDeviceBase()
{
	resetUsageData();
}

//--------------------------------------------------------------------------------
template <class T>
bool LibUSBDeviceBase<T>::release()
{
	bool result = T::release();
	resetUsageData();

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
void LibUSBDeviceBase<T>::initialize()
{
	START_IN_WORKING_THREAD(initialize)

	initializeUSBPort();
	setFreeUsageData();

	T::initialize();
}

//--------------------------------------------------------------------------------
template <class T>
bool LibUSBDeviceBase<T>::setFreeUsageData()
{
	MutexLocker lock(&mUsageDataGuard);

	auto it = std::find_if(mUsageData.begin(), mUsageData.end(), [&] (bool aFree) -> bool { return aFree; });

	if (it == mUsageData.end())
	{
		return false;
	}

	return setUsageData(it.key());
}

//--------------------------------------------------------------------------------
template <class T>
void LibUSBDeviceBase<T>::resetUsageData()
{
	MutexLocker lock(&mUsageDataGuard);

	libusb_device * device = mLibUSBPort.getDevice();

	if (mUsageData.contains(device))
	{
		mUsageData[device] = true;
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool LibUSBDeviceBase<T>::setUsageData(libusb_device * aDevice)
{
	CLibUSB::SDeviceProperties properties = mLibUSBPort.getDevicesProperties(false)[aDevice];
	QString logVID = properties.deviceData[CHardwareUSB::VID].toString();
	QString logPID = properties.deviceData[CHardwareUSB::PID].toString();

	if (!mDetectingData->data().contains(properties.VID))
	{
		toLog(LogLevel::Normal, QString("%1: Failed to set usage data due to no such VID %2").arg(mDeviceName).arg(logVID));
		return false;
	}

	QMap<quint16, CUSBDevice::SProductData> & PIDData = mDetectingData->value(properties.VID).data();

	if (!PIDData.contains(properties.PID))
	{
		toLog(LogLevel::Normal, QString("%1: Failed to set usage data due to no such PID %2 for VID %3").arg(mDeviceName).arg(logPID).arg(logVID));
		return false;
	}

	CUSBDevice::SProductData data = PIDData[properties.PID];
	mDeviceName = data.model;
	mVerified = data.verified;

	toLog(LogLevel::Normal, QString("%1: Set usage data for device with VID %2 and PID %3, %4").arg(mDeviceName).arg(logVID).arg(logPID).arg(properties.portData));

	mUsageData[aDevice] = false;
	mLibUSBPort.setDevice(aDevice);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void LibUSBDeviceBase<T>::initializeUSBPort()
{
	mLibUSBPort.initialize();
	CLibUSB::TDeviceProperties devicesProperties = mLibUSBPort.getDevicesProperties(true);

	MutexLocker lock(&mUsageDataGuard);

	QSet<libusb_device *> deletedDevices = mUsageData.keys().toSet() - devicesProperties.keys().toSet();

	foreach(libusb_device * device, deletedDevices)
	{
		mUsageData.remove(device);
	}

	for (auto it = devicesProperties.begin(); it != devicesProperties.end(); ++it)
	{
		for (auto jt = mDetectingData->data().begin(); jt != mDetectingData->data().end(); ++jt)
		{
			for (auto kt = jt.value().data().begin(); kt != jt.value().data().end(); ++kt)
			{
				if (!mUsageData.contains(it.key()) && (it->VID == jt.key()) && (it->PID == kt.key()))
				{
					mUsageData.insert(it.key(), true);
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool LibUSBDeviceBase<T>::checkConnectionAbility()
{
	return checkError(IOPortStatusCode::Error::Busy, [&] () -> bool { return mIOPort->open(); }, "device cannot open port");
}

//--------------------------------------------------------------------------------
template <class T>
bool LibUSBDeviceBase<T>::checkPort()
{
	if (mIOPort->isExist())
	{
		return true;
	}
	else if (!mIOPort->deviceConnected())
	{
		return false;
	}

	initializeUSBPort();

	return setFreeUsageData() && checkExistence();
}

//--------------------------------------------------------------------------------
template <class T>
void LibUSBDeviceBase<T>::postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	if (mIOPort && aNewStatusCollection.contains(DeviceStatusCode::Error::NotAvailable))
	{
		mIOPort->close();
	}

	T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//------------------------------------------------------------------------------
template <class T>
IDevice::IDetectingIterator * LibUSBDeviceBase<T>::getDetectingIterator()
{
	initializeUSBPort();

	mDetectingPosition = -1;

	return mUsageData.size() > 0 ? this : nullptr;
}

//--------------------------------------------------------------------------------
template <class T>
bool LibUSBDeviceBase<T>::find()
{
	if ((mDetectingPosition < 0) || (mDetectingPosition >= mUsageData.size()))
	{
		return false;
	}

	{
		MutexLocker lock(&mUsageDataGuard);

		auto it = mUsageData.begin() + mDetectingPosition;

		if (!setUsageData(it.key()))
		{
			return false;
		}
	}

	return checkExistence();
}

//------------------------------------------------------------------------------
template <class T>
bool LibUSBDeviceBase<T>::moveNext()
{
	mDetectingPosition++;

	return (mDetectingPosition >= 0) && (mDetectingPosition < mUsageData.size());
}

//--------------------------------------------------------------------------------
