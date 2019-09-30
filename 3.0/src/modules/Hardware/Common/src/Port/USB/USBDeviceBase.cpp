/* @file Базовый класс устройств на USB-порту. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QWriteLocker>
#include <QtCore/QReadLocker>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/CardReaders/ProtoMifareReader.h"
#include "Hardware/HID/ProtoHID.h"

// Project
#include "USBDeviceBase.h"

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
#define INSTANCE_USB_DEVICE(aClass) template class aClass; aClass::TPDOData aClass::mPDOData; QMutex aClass::mPDODataGuard(QMutex::Recursive);

INSTANCE_USB_DEVICE(USBDeviceBase<PortPollingDeviceBase<ProtoMifareReader>>)
INSTANCE_USB_DEVICE(USBDeviceBase<PortPollingDeviceBase<ProtoHID>>)

//--------------------------------------------------------------------------------
template <class T>
USBDeviceBase<T>::USBDeviceBase() : mPDODetecting(false), mPortUsing(true)
{
	mIOPort = &mUSBPort;

	mDetectingData = CUSBDevice::PDetectingData(new CUSBDevice::DetectingData());
	mReplaceableStatuses << DeviceStatusCode::Error::PowerSupply;
}

//--------------------------------------------------------------------------------
template <class T>
USBDeviceBase<T>::~USBDeviceBase()
{
	resetPDOName();
}

//--------------------------------------------------------------------------------
template <class T>
bool USBDeviceBase<T>::release()
{
	bool result = T::release();
	resetPDOName();

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
void USBDeviceBase<T>::initialize()
{
	START_IN_WORKING_THREAD(initialize)

	initializeUSBPort();
	setFreePDOName();

	T::initialize();
}

//--------------------------------------------------------------------------------
template <class T>
bool USBDeviceBase<T>::setFreePDOName()
{
	MutexLocker lock(&mPDODataGuard);

	auto it = std::find_if(mPDOData.begin(), mPDOData.end(), [&](bool aFree) -> bool { return aFree; });

	if (it == mPDOData.end())
	{
		return false;
	}

	return setPDOName(it.key());
}

//--------------------------------------------------------------------------------
template <class T>
void USBDeviceBase<T>::resetPDOName()
{
	MutexLocker lock(&mPDODataGuard);

	QString PDOName = mIOPort->getDeviceConfiguration()[CHardwareSDK::SystemName].toString();

	if (mPDOData.contains(PDOName))
	{
		mPDOData[PDOName] = true;
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool USBDeviceBase<T>::setPDOName(const QString & aPDOName)
{
	SWinDeviceProperties properties = mUSBPort.getDevicesProperties(false, true)[aPDOName];
	QString logVID = ProtocolUtils::toHexLog(properties.VID);
	QString logPID = ProtocolUtils::toHexLog(properties.PID);

	if (!mDetectingData->data().contains(properties.VID))
	{
		toLog(LogLevel::Normal, QString("%1: Failed to set PDO name due to no such VID %2").arg(mDeviceName).arg(logVID));
		return false;
	}

	QMap<quint16, CUSBDevice::SProductData> & PIDData = mDetectingData->value(properties.VID).data();

	if (!PIDData.contains(properties.PID))
	{
		toLog(LogLevel::Normal, QString("%1: Failed to set PDO name due to no such PID %2 for VID %3").arg(mDeviceName).arg(logPID).arg(logVID));
		return false;
	}

	CUSBDevice::SProductData data = PIDData[properties.PID];
	mDeviceName = data.model;
	mVerified = data.verified;

	mPDOData[aPDOName] = false;

	QVariantMap configuration;
	configuration.insert(CHardwareSDK::SystemName, aPDOName);
	toLog(LogLevel::Normal, QString("%1: Set USB PDO name %2, VID %3, PID %4").arg(mDeviceName).arg(aPDOName).arg(logVID).arg(logPID));

	mIOPort->setDeviceConfiguration(configuration);

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void USBDeviceBase<T>::initializeUSBPort()
{
	mUSBPort.initialize();
	TWinDeviceProperties devicesProperties = mUSBPort.getDevicesProperties(true, mPDODetecting);

	MutexLocker lock(&mPDODataGuard);

	QSet<QString> deletedPDONames = mPDOData.keys().toSet() - devicesProperties.keys().toSet();

	foreach(const QString & aPDOName, deletedPDONames)
	{
		mPDOData.remove(aPDOName);
	}

	for (auto it = devicesProperties.begin(); it != devicesProperties.end(); ++it)
	{
		for (auto jt = mDetectingData->data().begin(); jt != mDetectingData->data().end(); ++jt)
		{
			for (auto kt = jt.value().data().begin(); kt != jt.value().data().end(); ++kt)
			{
				if (!mPDOData.contains(it.key()) && (it->VID == jt.key()) && (it->PID == kt.key()))
				{
					mPDOData.insert(it.key(), true);
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------
template <class T>
bool USBDeviceBase<T>::checkConnectionAbility()
{
	return !mPortUsing || checkError(IOPortStatusCode::Error::Busy, [&] () -> bool { return mIOPort->open(); }, "device cannot open port");
}

//--------------------------------------------------------------------------------
template <class T>
bool USBDeviceBase<T>::checkPort()
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

	return setFreePDOName() && checkExistence();
}

//--------------------------------------------------------------------------------
template <class T>
void USBDeviceBase<T>::postPollingAction(const TStatusCollection & aNewStatusCollection, const TStatusCollection & aOldStatusCollection)
{
	if (mIOPort && mPortUsing && aNewStatusCollection.contains(DeviceStatusCode::Error::NotAvailable))
	{
		mIOPort->close();
	}

	T::postPollingAction(aNewStatusCollection, aOldStatusCollection);
}

//------------------------------------------------------------------------------
template <class T>
IDevice::IDetectingIterator * USBDeviceBase<T>::getDetectingIterator()
{
	initializeUSBPort();

	mDetectingPosition = -1;

	return mPDOData.size() > 0 ? this : nullptr;
}

//--------------------------------------------------------------------------------
template <class T>
bool USBDeviceBase<T>::find()
{
	if ((mDetectingPosition < 0) || (mDetectingPosition >= mPDOData.size()))
	{
		return false;
	}

	{
		MutexLocker lock(&mPDODataGuard);

		auto it = mPDOData.begin() + mDetectingPosition;

		if (!setPDOName(it.key()))
		{
			return false;
		}
	}

	return checkExistence();
}

//------------------------------------------------------------------------------
template <class T>
bool USBDeviceBase<T>::moveNext()
{
	mDetectingPosition++;

	return (mDetectingPosition >= 0) && (mDetectingPosition < mPDOData.size());
}

//--------------------------------------------------------------------------------
