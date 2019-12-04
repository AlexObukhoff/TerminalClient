/* @file Инстанцирование шаблонов виртуальных устройств. */

#include "../../../../modules/Hardware/CashAcceptors/src/CashAcceptorBase.cpp"
#include "../../../../modules/Hardware/CashDispensers/src/DispenserBase.cpp"

//------------------------------------------------------------------------------
template class CashAcceptorBase<DeviceBase<ProtoCashAcceptor>>;
template class DispenserBase<DeviceBase<ProtoDispenser>>;

//------------------------------------------------------------------------------
