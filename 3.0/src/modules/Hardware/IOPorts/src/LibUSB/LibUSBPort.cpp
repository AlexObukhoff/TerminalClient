/* @file LibUSB-порт. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

// Project
#include "LibUSBPort.h"
#include "LibUSBUtils.h"

using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
QMutex LibUSBPort::mDevicesPropertyMutex(QMutex::Recursive);

//--------------------------------------------------------------------------------
LibUSBPort::LibUSBPort(): mHandle(nullptr), mExist(false), mDevice(nullptr)
{
	mType = SDK::Driver::EPortTypes::USB;
	mInitializationError = true;
}

//--------------------------------------------------------------------------------
void LibUSBPort::setDevice(libusb_device * aDevice)
{
	mDevice = aDevice;
	mDeviceProperties = getDevicesProperties(false)[aDevice];

	auto getEPLogData = [&] (const CLibUSB::SEndPoint & aEP) -> QString { return QString("max packet size = %1, data = %2, transfer type = %3")
		.arg(aEP.maxPacketSize).arg(toHexLog(aEP.data)).arg(CLibUSBUtils::TransferTypeDescriptions[uint8_t(aEP.transferType)]); };

	if (!mDeviceProperties.valid())
	{
		toLog(LogLevel::Error, QString("Port properties are wrong: VID = %1, PID = %2,\ndeviceToHost = %3,\nhostToDevice = %4")
			.arg(toHexLog(mDeviceProperties.VID))
			.arg(toHexLog(mDeviceProperties.PID))
			.arg(getEPLogData(mDeviceProperties.deviceToHost))
			.arg(getEPLogData(mDeviceProperties.hostToDevice)));
	}
}

//--------------------------------------------------------------------------------
libusb_device * LibUSBPort::getDevice() const
{
	return mDevice;
}

//--------------------------------------------------------------------------------
void LibUSBPort::initialize()
{
	const libusb_version * versionData = libusb_get_version();

	if (versionData)
	{
		QString RCVersion = versionData->rc ? QString(versionData->rc).simplified() : "";
		QString description = versionData->describe ? QString(versionData->describe).simplified() : "";
		QString data = QString("version %1.%2.%3.%4%5%6").arg(versionData->major).arg(versionData->minor).arg(versionData->micro).arg(versionData->nano)
			.arg(RCVersion.isEmpty() ? "" : (" " + RCVersion))
			.arg(description.isEmpty() ? "" : QString(" (%1)").arg(description));

		setDeviceParameter(CHardwareSDK::LibraryVersion, data);
	}

	mInitializationError = !LibUSBUtils::getContext(mLog);

	if (mInitializationError)
	{
		return;
	}

	CLibUSB::TDeviceProperties libUSBProperties = getDevicesProperties(false);

	QStringList mineData;
	QStringList otherData;

	for (auto it = libUSBProperties.begin(); it != libUSBProperties.end(); ++it)
	{
		QStringList & data = (it.key() == mDevice) ? mineData : otherData;
		data << LibUSBUtils::getPropertyLog(it->deviceData);
	}

	adjustData(mineData, otherData);
}

//--------------------------------------------------------------------------------
bool LibUSBPort::release()
{
	bool closingResult = close();

	LibUSBUtils::releaseDeviceList();
	LibUSBUtils::releaseContext(mLog);

	bool result = MetaDevice::release();

	return closingResult && result;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::open()
{
	if (mHandle)
	{
		return true;
	}

	if (mInitializationError || !mDevice)
	{
		return false;
	}

	setConfigParameter(CHardware::Port::JustConnected, false);

	if (!LIB_USB_CALL_LOG(mLog, libusb_open, mDevice, &mHandle))
	{
		return false;
	}

	CLibUSB::SDeviceProperties deviceProperties = getDevicesProperties(false)[mDevice];
	QVariantMap & deviceData = deviceProperties.deviceData;

	auto getData = [&deviceData] (const QString & aKey) -> QString { return deviceData.value(aKey).toString().simplified(); };
	auto getFullData = [&deviceData, &getData] (const QString & aKey1, const QString & aKey2) -> QString
		{ QString result = QString("%1 = %2").arg(aKey1.toUpper()).arg(getData(aKey1)); QString option = getData(aKey2);
			return option.isEmpty() ? result : QString("%1 (%2)").arg(result).arg(option); };

	toLog(LogLevel::Normal, QString("%1 with device %2 and %3 is opened")
		.arg(deviceProperties.portData)
		.arg(getFullData(CHardwareUSB::VID, DeviceUSBData::Vendor))
		.arg(getFullData(CHardwareUSB::PID, DeviceUSBData::Product)));

	int existingConfiguration = -1;

	if (!LIB_USB_CALL(libusb_get_configuration, mHandle, &existingConfiguration))
	{
		return false;
	}

	if ((existingConfiguration != 1) && !LIB_USB_CALL(libusb_set_configuration, mHandle, 1))
	{
		return false;
	}

	LIB_USB_CALL(libusb_set_auto_detach_kernel_driver, mHandle, 1);

	if (!LIB_USB_CALL(libusb_claim_interface, mHandle, 0))
	{
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::close()
{
	bool result = true;
	bool beenOpened = mHandle;

	if (!mInitializationError && mHandle)
	{
		result = LIB_USB_CALL(libusb_release_interface, mHandle, 0);
		libusb_close(mHandle);
	}

	if (result && beenOpened)
	{
		toLog(LogLevel::Normal, QString("Port %1 is closed.").arg(getDevicesProperties(false)[mDevice].portData));
	}

	mHandle = nullptr;

	return result;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::checkExistence()
{
	if (isExist())
	{
		return true;
	}

	mExist = mDevices.contains(mDevice);

	if (!mExist)
	{
		if (mDevice)
		{
			toLog(LogLevel::Error, "Port does not exist.");
		}

		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::checkReady()
{
	if (!checkExistence())
	{
		return false;
	}

	if (!mHandle && !open())
	{
		toLog(LogLevel::Error, "Port does not opened.");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::read(QByteArray & aData, int aTimeout, int aMinSize)
{
	aData.clear();

	if (!checkReady())
	{
		return false;
	}

	QTime waitingTimer;
	waitingTimer.start();

	while ((waitingTimer.elapsed() < aTimeout) && (aData.size() < aMinSize))
	{
		int received = 0;
		CLibUSB::SEndPoint & EP = mDeviceProperties.deviceToHost;
		mReadingBuffer.fill(ASCII::NUL, EP.maxPacketSize);

		TResult result = LIB_USB_CALL(mDeviceProperties.hostToDevice.processIO, mHandle, EP.data, (unsigned char *)&mReadingBuffer[0], EP.maxPacketSize, &received, aTimeout);

		if (LIB_USB_SUCCESS(result))
		{
			aData.append(mReadingBuffer.data(), received);
		}
		else if (result != LIBUSB_ERROR_TIMEOUT)
		{
			return false;
		}
	}

	if (mDeviceIOLoging == ELoggingType::ReadWrite)
	{
		toLog(LogLevel::Normal, QString("%1: << {%2}").arg(mConnectedDeviceName).arg(aData.toHex().constData()));
	}

	return true;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::write(const QByteArray & aData)
{
	if (aData.isEmpty())
	{
		toLog(LogLevel::Normal, mConnectedDeviceName + ": written data is empty.");
		return false;
	}

	if (!checkReady())
	{
		return false;
	}

	if (mDeviceIOLoging != ELoggingType::None)
	{
		toLog(LogLevel::Normal, QString("%1: >> {%2}").arg(mConnectedDeviceName).arg(aData.toHex().constData()));
	}

	int partSize = mDeviceProperties.hostToDevice.maxPacketSize;
	int parts = qCeil(double(aData.size()) / partSize);

	for (int i = 0; i < parts; ++i)
	{
		if (!performWrite(aData.mid(i * partSize, partSize)))
		{
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::performWrite(const QByteArray & aData)
{
	int bytesWritten = 0;
	int actualSize = aData.size();
	int timeout = CLibUSBPort::writingTimeout(actualSize);

	TResult result = LIB_USB_CALL(mDeviceProperties.deviceToHost.processIO, mHandle, mDeviceProperties.hostToDevice(), (unsigned char *)aData.data(), actualSize, &bytesWritten, timeout);

	if (!result)
	{
		return false;
	}

	if (bytesWritten != actualSize)
	{
		toLog(LogLevel::Normal, mConnectedDeviceName + QString(": %1 bytes instead of %2 bytes have been written.").arg(bytesWritten).arg(actualSize));
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
TResult LibUSBPort::handleResult(const QString & aFunctionName, int aResult)
{
	TResult result = LibUSBUtils::logAnswer(aFunctionName, aResult, mLog);

	if (CLibUSBPort::DisappearingErrors.contains(aResult) && !getDevicesProperties(true).contains(mDevice))
	{
		close();

		mExist = false;
	}

	return result;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::deviceConnected()
{
	CLibUSB::TDeviceProperties devicesProperties = getDevicesProperties(true);
	int result = (devicesProperties.size() - mDevicesProperties.size()) * mDevicesProperties.size();

	mDevicesProperties = devicesProperties;

	if (result > 0)
	{
		setConfigParameter(CHardware::Port::JustConnected, true);

		return true;
	}

	checkExistence();

	return false;
};

//--------------------------------------------------------------------------------
bool LibUSBPort::isExist()
{
	return mExist;
}

//--------------------------------------------------------------------------------
CLibUSB::TDeviceProperties LibUSBPort::getDevicesProperties(bool aForce)
{
	QMutexLocker locker(&mDevicesPropertyMutex);

	static CLibUSB::TDeviceProperties properties;

	if ((!properties.isEmpty() && !aForce) || !LibUSBUtils::getDevicesProperties(properties, aForce))
	{
		return properties;
	}

	for (auto it = properties.begin(); it != properties.end();)
	{
		QString deviceProduct = it->deviceData.value(DeviceUSBData::Product).toString().toLower();
		bool needErase = !it->VID || !it->PID || deviceProduct.contains("mouse");
		it = needErase ? properties.erase(it) : it + 1;
	}

	mDevices = properties.keys();

	return properties;
}

//--------------------------------------------------------------------------------
