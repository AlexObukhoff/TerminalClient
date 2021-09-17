/* @file Базовые принтеры с портовыми реализациями протоколов. */

#pragma once

#include "../../../modules/Hardware/Printers/src/Base/Port/PortPrinterBase.h"

//--------------------------------------------------------------------------------
template <class T>
class SerialPrinterBase : public PortPrinterBase<T>
{
	SET_VCOM_DATA(None, ConnectionTypes::COMOnly, None)

public:
	/// Получение списка настроек порта, необязательных для редактирования пользователем.
	static QStringList getOptionalPortSettings()
	{
		return QStringList()
			<< COMPortSDK::Parity
			<< COMPortSDK::ByteSize
			<< COMPortSDK::StopBits
			<< COMPortSDK::RTS
			<< COMPortSDK::DTR;
	}
};

//--------------------------------------------------------------------------------
typedef SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>> TSerialPrinterBase;
typedef   PortPrinterBase<PrinterBase<LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>> TLibUSBPrinterBase;
typedef   PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>> TTCPPrinterBase;

//--------------------------------------------------------------------------------
