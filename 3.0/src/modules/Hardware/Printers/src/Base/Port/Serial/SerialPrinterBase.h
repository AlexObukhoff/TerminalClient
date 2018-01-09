/* @file Базовый принтер с COM-портовой реализацией протокола. */

#pragma once

// Modules
#include "Hardware/FR/ProtoFR.h"

// Project
#include "Hardware/Printers/PortPrinterBase.h"

//--------------------------------------------------------------------------------
template <class T>
class SerialPrinterBase : public PortPrinterBase<T>
{
public:
	/// Получение списка настроек порта, необязательных для редактирования пользователем.
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

typedef SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>> TSerialPrinterBase;

//--------------------------------------------------------------------------------
