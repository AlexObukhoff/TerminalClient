/* @file Набор утилит для работы с LibUSB. */

#pragma once

// Common
#include <Common/ILogable.h>

// Modules
#include "Hardware/Common/HardwareConstants.h"
#include "Hardware/Common/CommandResults.h"

// Project
#include "Hardware/IOPorts/LibUSBDeviceDataTypes.h"

//--------------------------------------------------------------------------------
namespace CLibUSBUtils
{
	/// Русская локаль для вывода сообщений.
	const char Locale[] = "ru";

	/// Размер буфера для получения значения по дескриптору.
	const int DescriptorDataSize = 1024;

	/// ID USB 2.01 в BCD.
	const uint16_t USB2_01 = 0x0201;

	//--------------------------------------------------------------------------------
	class CSpeedDescriptions: public CDescription<uint16_t>
	{
	public:
		CSpeedDescriptions()
		{
			append(LIBUSB_LOW_SPEED_OPERATION,   "1.5 MBit/s");
			append(LIBUSB_FULL_SPEED_OPERATION,  "12 MBit/s");
			append(LIBUSB_HIGH_SPEED_OPERATION,  "480 MBit/s");
			append(LIBUSB_SUPER_SPEED_OPERATION, "5 GBit/s");
			append(LIBUSB_SPEED_SUPER_PLUS,      "10 GBit/s");
		}

	protected:
		virtual QString value(const uint16_t & aKey) const
		{
			return mBuffer.contains(aKey) ? mBuffer.value(aKey, mDefaultValue) : QString("%1 (unknown)").arg(aKey);
		}
	};

	static CSpeedDescriptions SpeedDescriptions;

	//--------------------------------------------------------------------------------
	inline libusb_transfer_type transferType(uint8_t aData) { return libusb_transfer_type(aData & LIBUSB_TRANSFER_TYPE_MASK); }

	class CTransferTypeDescriptions: public CLibUSB::CConfigTypeDescriptions<libusb_transfer_type, 0>
	{
	public:
		CTransferTypeDescriptions()
		{
			add(LIBUSB_TRANSFER_TYPE_CONTROL,     "control");
			add(LIBUSB_TRANSFER_TYPE_ISOCHRONOUS, "isochronous");
			add(LIBUSB_TRANSFER_TYPE_BULK,        "bulk");
			add(LIBUSB_TRANSFER_TYPE_INTERRUPT,   "interrupt");
			add(LIBUSB_TRANSFER_TYPE_BULK_STREAM, "stream");  // ??
		}
	};

	static CTransferTypeDescriptions TransferTypeDescriptions;

	//--------------------------------------------------------------------------------
	class CIsoSyncTypeDescriptions: public CLibUSB::CConfigTypeDescriptions<libusb_iso_sync_type, 2>
	{
	public:
		CIsoSyncTypeDescriptions()
		{
			add(LIBUSB_ISO_SYNC_TYPE_NONE,     "none");
			add(LIBUSB_ISO_SYNC_TYPE_ASYNC,    "asynchronous");
			add(LIBUSB_ISO_SYNC_TYPE_ADAPTIVE, "adaptive");
			add(LIBUSB_ISO_SYNC_TYPE_SYNC,     "synchronous");
		}
	};

	static CIsoSyncTypeDescriptions IsoSyncTypeDescriptions;

	//--------------------------------------------------------------------------------
	class CIsoUsageTypeDescriptions: public CLibUSB::CConfigTypeDescriptions<libusb_iso_usage_type, 4>
	{
	public:
		CIsoUsageTypeDescriptions()
		{
			add(LIBUSB_ISO_USAGE_TYPE_DATA,     "data");
			add(LIBUSB_ISO_USAGE_TYPE_FEEDBACK, "feedback");
			add(LIBUSB_ISO_USAGE_TYPE_IMPLICIT, "implicit feedback data");
		}
	};

	static CIsoUsageTypeDescriptions IsoUsageTypeDescriptions;
}

#define LIB_USB_SUCCESS(result) result == LIBUSB_SUCCESS
#define LIB_USB_CALL_DEBUG(aName, ...) logAnswer(#aName, aName(__VA_ARGS__), nullptr)
#define LIB_USB_CALL_LOG(aLog, aName, ...) LibUSBUtils::logAnswer(#aName, aName(__VA_ARGS__), aLog)

//--------------------------------------------------------------------------------
namespace LibUSBUtils
{
	/// Логгировать ошибку.
	TResult logAnswer(const QString & aFunctionName, int aResult, ILog * aLog = nullptr);

	/// Получить контекст библиотеки LibUSB.
	libusb_context * getContext(ILog * aLog = nullptr);

	/// Освободить контекст библиотеки LibUSB.
	void releaseContext(ILog * aLog);

	/// Получить список устройств.
	typedef QList<libusb_device *> TDeviceList;
	bool getDeviceList(TDeviceList & aList, bool aForce = false);
	libusb_device ** getDeviceList(ssize_t & aSize, bool aForce = false);

	/// Освободить список устройств.
	void releaseDeviceList();

	/// Получить лог для печати параметров устройств.
	QString getPropertyLog(const QVariantMap & aData);
	QString getPropertyLog(const CLibUSB::TDeviceDataList & aList, int aIndex = 0);

	/// Получить данные о ресурсах устройств.
	bool getDevicesProperties(CLibUSB::TDeviceProperties & aDeviceProperties, bool aForce = false);

	/// Получить данные о ресурсах устройства.
	CLibUSB::SDeviceProperties getDevicesProperties(libusb_device * aDevice);

	/// Получить данные устройства по его дескриптору.
	void getDeviceDescriptorData(libusb_device * aDevice, const libusb_device_descriptor & aDescriptor, CLibUSB::SDeviceProperties & aProperties);

	/// Получить данные BOS (Binary Object Store).
	QVariantMap getBOSData(libusb_device_handle * aDeviceHandle);

	/// Получить данные конфигураций.
	TLibUSBDataList getConfigData(libusb_device * aDevice, const libusb_device_descriptor & aDeviceDescriptor);

	/// Получить данные интерфейсов.
	TLibUSBDataList getInterfaceData(libusb_config_descriptor * config);

	/// Получить данные конечных точек (endpoint).
	TLibUSBDataList getEndpointData(const libusb_interface_descriptor & aInterface);

	/// Получить данные экстра-дескрипторов конечных точек (endpoint).
	TLibUSBDataList getEPCompanionData(const libusb_endpoint_descriptor & aEndpoint);

	/// Получить контекст для библиотеки LibUSB.
	void addDescriptorData(libusb_device_handle * aHandle, uint8_t aDescriptor, QVariant & aData);

	/// Получить набор по ключу из списка набора данных.
	bool getDataFromMap(QVariantMap & aData, const QString & aKey, bool aSetNextData = true);

	/// Конвертировать число в строку с точкой посередине xx.yy.
	template <class T>
	QString doubleBCD2String(T aData);
}

namespace DeviceUSBData = CDeviceData::Ports::USB;
namespace CHardwareUSB = CHardware::Port::USB;

//--------------------------------------------------------------------------------
