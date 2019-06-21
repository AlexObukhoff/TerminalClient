/* @file Асинхронная Windows-реализация USB-порта. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/QtGlobal>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// Project
#include "USBPort.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
QMutex USBPort::mSystemPropertyMutex(QMutex::Recursive);

//--------------------------------------------------------------------------------
USBPort::USBPort()
{
	mType = EPortTypes::USB;
	mUuids = CUSBPort::Uuids();
	mPathProperty = CUSBPort::PathProperty;
	setOpeningTimeout(CAsyncSerialPort::OpeningTimeout + CUSBPort::OpeningPause);
}

//--------------------------------------------------------------------------------
bool USBPort::performOpen()
{
	if (!checkReady())
	{
		return false;
	}

	SleepHelper::msleep(CUSBPort::OpeningPause);

	QByteArray fileName = getDevicesProperties(false)[mSystemName].path.toLatin1();
	mPortHandle = CreateFileA(fileName.data(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

	if (mPortHandle == INVALID_HANDLE_VALUE)
	{
		handleError("CreateFileA");
	}
	else
	{
		BOOL_CALL(SetCommMask, EV_ERR | EV_RXCHAR);

		::RtlSecureZeroMemory(&mReadOverlapped, sizeof(mReadOverlapped));
		mReadOverlapped.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

		::RtlSecureZeroMemory(&mWriteOverlapped, sizeof(mWriteOverlapped));
		mWriteOverlapped.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

		return true;
	}

	close();

	return false;
}

//--------------------------------------------------------------------------------
bool USBPort::checkReady()
{
	if (mExist)
	{
		return true;
	}

	mExist = mSystemNames.contains(mSystemName);

	if (!mExist)
	{
		setOpeningTimeout(CAsyncSerialPort::OnlineOpeningTimeout);

		toLog(LogLevel::Error, QString("Port %1 does not exist.").arg(mSystemName));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool USBPort::clear()
{
	// USB порту чистка буферов не требуется
	return true;
}

//--------------------------------------------------------------------------------
bool USBPort::processReading(QByteArray & aData, int aTimeout)
{
	DWORD result = 0;
	aData.clear();

	QMutexLocker locker(&mReadMutex);

	if (!checkReady() || !waitAsyncAction(result, aTimeout))
	{
		return false;
	}

	if (result == WAIT_OBJECT_0)
	{
		mReadBytes = 0;
		::GetOverlappedResult(mPortHandle, &mReadOverlapped, &mReadBytes, TRUE);
	}

	if (mReadingBuffer.isEmpty())
	{
		mReadingBuffer.fill(ASCII::NUL, CUSBPort::DefaultMaxReadSize);
	}

	if (mReadingBuffer != CUSBPort::EmptyBuffer)
	{
		aData.append(mReadingBuffer.data(), mMaxReadingSize);

		mReadingBuffer.fill(ASCII::NUL, CUSBPort::DefaultMaxReadSize);
	}

	::CancelIo(mPortHandle);

	mReadBytes = 0;
	::ReadFile(mPortHandle, &mReadingBuffer[0], CUSBPort::DefaultMaxReadSize, &mReadBytes, &mReadOverlapped);

	return true;
}

//--------------------------------------------------------------------------------
TWinDeviceProperties USBPort::getDevicesProperties(bool aForce, bool aPDODetecting)
{
	QMutexLocker locker(&mSystemPropertyMutex);

	static TWinDeviceProperties properties;

	if (!properties.isEmpty() && !aForce)
	{
		return properties;
	}

	properties = getDeviceProperties(CUSBPort::Uuids(), CUSBPort::PathProperty);
	DeviceWinProperties deviceWinProperties;

	for (auto it = properties.begin(); it != properties.end();)
	{
		auto getProperty = [&] (DWORD aCode) -> QString { return it->data[deviceWinProperties[aCode]]; };

		bool erase = getProperty(SPDRP_ENUMERATOR_NAME).contains(CUSBPort::DeviceTags::ACPI) ||
		             getProperty(SPDRP_CLASS).contains(CUSBPort::DeviceTags::Mouse) ||
		            (getProperty(SPDRP_PHYSICAL_DEVICE_OBJECT_NAME).contains(CUSBPort::DeviceTags::USBPDO) && !aPDODetecting);

		it = erase ? properties.erase(it) : it + 1;
	}

	mSystemNames = properties.keys();

	return properties;
}

//--------------------------------------------------------------------------------
