/* @file Дефолтное HID-устройство на COM-порту. */

#pragma once

// Modules
#include "Hardware/Common/SerialDeviceBase.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/HID/ProtoHID.h"

// Project
#include "PortScanner.h"

//--------------------------------------------------------------------------------
typedef PortScanner<SerialDeviceBase<PortPollingDeviceBase<ProtoHID>>> TSerialScanner;

class SerialScanner: public TSerialScanner
{
public:
	SerialScanner();

protected:
	/// Получить данные
	virtual bool getData(QByteArray & aAnswer);
};

//--------------------------------------------------------------------------------
