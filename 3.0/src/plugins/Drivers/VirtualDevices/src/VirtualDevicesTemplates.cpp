/* @file Инстанцирование шаблонов виртуальных устройств. */

#include "../../../../modules/Hardware/CashAcceptors/src/CashAcceptorBase.cpp"
#include "../../../../modules/Hardware/CashDispensers/src/DispenserBase.cpp"
#include "../../../../modules/Hardware/Printers/src/Base/PrinterBase.cpp"

//------------------------------------------------------------------------------
template class CashAcceptorBase<DeviceBase<ProtoCashAcceptor>>;
template class DispenserBase<DeviceBase<ProtoDispenser>>;
template class PrinterBase<PollingDeviceBase<DeviceBase<ProtoPrinter>>>;

//------------------------------------------------------------------------------
