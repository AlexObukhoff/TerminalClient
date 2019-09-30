/* @file POS-принтер на COM-порту. */

#pragma once

#include "POSPrinter.h"

//--------------------------------------------------------------------------------
template <class T>
class SerialPOSPrinter: public T
{
public:
	SerialPOSPrinter()
	{
		using namespace SDK::Driver::IOPort::COM;

		// параметры порта
		mPortParameters.clear();
		mPortParameters.insert(EParameters::BaudRate, POSPrinters::TSerialDevicePortParameter()
			<< EBaudRate::BR115200
			<< EBaudRate::BR19200
			<< EBaudRate::BR57600
			<< EBaudRate::BR38400
			<< EBaudRate::BR9600
			<< EBaudRate::BR4800);
		mPortParameters.insert(EParameters::Parity, POSPrinters::TSerialDevicePortParameter()
			<< EParity::No
			<< EParity::Even
			<< EParity::Odd);
		mPortParameters.insert(EParameters::ByteSize, POSPrinters::TSerialDevicePortParameter()
			<< 8);
		mPortParameters.insert(EParameters::RTS, POSPrinters::TSerialDevicePortParameter()
			<< ERTSControl::Toggle);
		mPortParameters.insert(EParameters::DTR, POSPrinters::TSerialDevicePortParameter()
			<< EDTRControl::Handshake);
	}
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<POSPrinter<TSerialPrinterBase>> TSerialPOSPrinter;
typedef                  POSPrinter<TLibUSBPrinterBase>  TLibUSBPOSPrinter;

//--------------------------------------------------------------------------------
