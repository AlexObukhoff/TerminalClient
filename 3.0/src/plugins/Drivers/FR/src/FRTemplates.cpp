/* @file Инстанцирование шаблонов фискальных регистраторов. */

#include "Hardware/Printers/PortPrintersBase.h"

#include "../../../../modules/Hardware/Common/src/OPOS/OPOSPollingDeviceBase.cpp"
#include "../../../../modules/Hardware/Printers/src/Base/PrinterBase.cpp"
#include "../../../../modules/Hardware/Printers/src/Base/Port/PortPrinterBase.cpp"
#include "../../../../modules/Hardware/FR/src/Base/FRBase.cpp"

//------------------------------------------------------------------------------
template class OPOSPollingDeviceBase<ProtoFR, OPOS::OPOSFiscalPrinter>;

template class PrinterBase<OPOSPollingDeviceBase<ProtoFR, OPOS::OPOSFiscalPrinter>>;
template class PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>;
template class PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>;

template class PortPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>;
template class PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>;

template class FRBase<PrinterBase<OPOSPollingDeviceBase<ProtoFR, OPOS::OPOSFiscalPrinter>>>;
template class FRBase<PortPrinterBase<PrinterBase<TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>;
template class FRBase<SerialPrinterBase<PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>>>>;

//------------------------------------------------------------------------------
