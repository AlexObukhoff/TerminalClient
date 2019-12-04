/* @file Инстанцирование шаблонов принтеров. */

#include "../../../../modules/Hardware/Common/src/Port/LibUSB/LibUSBDeviceBase.cpp"
#include "../../../../modules/Hardware/Printers/src/Base/PrinterBase.cpp"
#include "../../../../modules/Hardware/Printers/src/Base/Port/PortPrinterBase.cpp"

#include "../../../../modules/Hardware/Printers/src/POSPrinters/Common/POSPrinter.cpp"
#include "../../../../modules/Hardware/Printers/src/POSPrinters/EjectorPOS/EjectorPOS.cpp"
#include "../../../../modules/Hardware/Printers/src/POSPrinters/Custom/CustomPrinters.cpp"
#include "../../../../modules/Hardware/Printers/src/POSPrinters/Custom/CustomVKP/CustomVKP80.cpp"
#include "../../../../modules/Hardware/Printers/src/POSPrinters/Custom/CustomVKP/CustomVKP80III.cpp"
#include "../../../../modules/Hardware/Printers/src/POSPrinters/Citizen/CitizenPPU700/CitizenPPU700.cpp"

//-------------------------------------------------------------------------------
template class LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>;
template class PrinterBase<LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>;
template class PortPrinterBase<PrinterBase<LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>>;

template class POSPrinter<TLibUSBPrinterBase>;
template class EjectorPOS<TLibUSBPrinterBase>;
template class CustomVKP80<TLibUSBPrinterBase>;
template class CustomPrinter<TLibUSBPrinterBase>;
template class CustomVKP80III<TLibUSBPrinterBase>;
template class CitizenPPU700<TLibUSBPrinterBase>;
template class CitizenPPU700II<TLibUSBPrinterBase>;

//------------------------------------------------------------------------------
