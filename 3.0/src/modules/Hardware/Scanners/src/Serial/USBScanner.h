/* @file USB-сканер. */

#pragma once

// Modules
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/USBDeviceBase.h"
#include "Hardware/HID/ProtoHID.h"

// Project
#include "PortScanner.h"

//--------------------------------------------------------------------------------
namespace CUSBScanner
{
	/// Количество байтов в буфере для чтения ответа (независимо от наличия ответа) из USB-канала.
	const int USBAnswerSize = 100;
}

//--------------------------------------------------------------------------------
typedef PortScanner<USBDeviceBase<PortPollingDeviceBase<ProtoHID>>> TUSBScanner;

class USBScanner: public TUSBScanner
{
public:
	USBScanner();

	/// Возвращает список поддерживаемых устройств.
	static QStringList getModelList();

protected:
	/// Получить данные
	virtual bool getData(QByteArray & aAnswer);
};

//--------------------------------------------------------------------------------
