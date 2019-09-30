/* @file ������� �������� � ��������� ������������ ����������. */

#pragma once

#include "../../../modules/Hardware/Printers/src/Base/Port/PortPrinterBase.h"

//--------------------------------------------------------------------------------
template <class T>
class SerialPrinterBase : public PortPrinterBase<T>
{
public:
	/// ��������� ������ �������� �����, �������������� ��� �������������� �������������.
	static QStringList getOptionalPortSettings()
	{
		return QStringList()
			<< CHardware::Port::COM::Parity
			<< CHardware::Port::COM::ByteSize
			<< CHardware::Port::COM::StopBits
			<< CHardware::Port::COM::RTS
			<< CHardware::Port::COM::DTR;
	}
};

//--------------------------------------------------------------------------------
typedef SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>> TSerialPrinterBase;
typedef   PortPrinterBase<PrinterBase<LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>> TLibUSBPrinterBase;

//--------------------------------------------------------------------------------
