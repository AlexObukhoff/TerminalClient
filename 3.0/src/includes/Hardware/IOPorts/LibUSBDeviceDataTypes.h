/* @file Типы данных USB-устройств для использования в функционале LibUSB-порта. */

#pragma once

// LibUSB SDK
#pragma warning(push, 1)
#include "libusb/src/libusb/libusb.h"
#pragma warning(pop)

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVariantMap>
#include <QtCore/QMetaType>
#include <QtCore/QList>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/ASCII.h"

namespace CLibUSB
{
	/// Данные конечной точки.
	struct SEndPoint
	{
		libusb_transfer_type transferType;
		char data;
		int maxPacketSize;
		int pollingInterval;

		SEndPoint(): transferType(LIBUSB_TRANSFER_TYPE_CONTROL), data(ASCII::NUL), maxPacketSize(0), pollingInterval(0) {}
		SEndPoint(libusb_transfer_type aTransferType, char aData, int aMaxPacketSize, int aPollingInterval):
			transferType(aTransferType), data(aData), maxPacketSize(aMaxPacketSize), pollingInterval(aPollingInterval) {}

		bool getDirection() { return bool(data & LIBUSB_ENDPOINT_DIR_MASK); }
		bool valid() { return maxPacketSize && ~(data & (LIBUSB_ENDPOINT_DIR_MASK | LIBUSB_ENDPOINT_ADDRESS_MASK)) &&
			((transferType == LIBUSB_TRANSFER_TYPE_BULK) || (transferType == LIBUSB_TRANSFER_TYPE_INTERRUPT)); }
		char operator()() { return data; }
	};

	/// Параметры устройства.
	struct SDeviceProperties
	{
		QVariantMap deviceData;
		quint16 VID;
		quint16 PID;
		QString portData;
		SEndPoint deviceToHost;
		SEndPoint hostToDevice;
		bool valid() { return VID && PID && deviceToHost.valid() && hostToDevice.valid() && deviceToHost.getDirection() && !hostToDevice.getDirection(); }

		SDeviceProperties(): VID(0), PID(0) {}
		SDeviceProperties(const QVariantMap & aDeviceData): deviceData(aDeviceData) {}
	};

	typedef QMap<libusb_device *, SDeviceProperties> TDeviceProperties;
	typedef QList<QVariantMap> TDeviceDataList;

	//--------------------------------------------------------------------------------
	template <class T, int aIndex>
	class CConfigTypeDescriptions: public CDescription<T>
	{
	public:
		void add(T aKey, const QString & aDescription)
		{
			append(T(aKey << aIndex), aDescription);
		}

		const QString operator[] (uint8_t aData) const
		{
			return mBuffer.value(T(aData & ('\x03' << aIndex)), "unknown");
		}
	};
}

typedef QList<QVariantMap> TLibUSBDataList;
Q_DECLARE_METATYPE(TLibUSBDataList);

//--------------------------------------------------------------------------------
