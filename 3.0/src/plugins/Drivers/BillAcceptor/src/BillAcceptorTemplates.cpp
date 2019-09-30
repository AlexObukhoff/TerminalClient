/* @file Инстанцирование шаблонов купюроприемников. */

#include "Hardware/CashAcceptors/SerialCashAcceptor.h"

#include "../../../../modules/Hardware/CashAcceptors/src/CashAcceptorBase.cpp"
#include "../../../../modules/Hardware/CashAcceptors/src/PortCashAcceptor.cpp"
#include "../../../../modules/Hardware/CashDevices/src/CCTalk/CCTalkDeviceBase.cpp"
#include "../../../../modules/Hardware/Acceptors/src/CCTalk/CCTalkAcceptorBase.cpp"

//------------------------------------------------------------------------------
template class CashAcceptorBase<SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>>;
template class PortCashAcceptor<SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>>;
template class CCTalkDeviceBase<TSerialCashAcceptor>;
template class CCTalkAcceptorBase<TSerialCashAcceptor>;

//------------------------------------------------------------------------------
