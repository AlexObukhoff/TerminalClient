/* @file Инстанцирование шаблонов монетоприемников. */

#include "../../../../modules/Hardware/CashAcceptors/src/CashAcceptorBase.cpp"
#include "../../../../modules/Hardware/CashAcceptors/src/PortCashAcceptor.cpp"

#include "../../../../modules/Hardware/CashDevices/src/CCTalk/CCTalkDeviceBase.cpp"
#include "../../../../modules/Hardware/Acceptors/src/CCTalk/CCTalkAcceptorBase.cpp"
#include "../../../../modules/Hardware/Acceptors/src/CCTalk/CCTalkComplexEnableAcceptor.cpp"
#include "../../../../modules/Hardware/CoinAcceptors/src/CoinAcceptorBase.cpp"
#include "../../../../modules/Hardware/CoinAcceptors/src/CCTalk/CCTalkCoinAcceptorBase.cpp"

//------------------------------------------------------------------------------
template class CashAcceptorBase<SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>>;
template class PortCashAcceptor<SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>>;

template class CCTalkDeviceBase<CoinAcceptorBase>;
template class CCTalkDeviceBase<CCTalkCoinAcceptorBase>;
template class CCTalkAcceptorBase<CoinAcceptorBase>;
template class CCTalkAcceptorBase<CCTalkCoinAcceptorBase>;
template class CCTalkComplexEnableAcceptor<CCTalkCoinAcceptorBase>;

//------------------------------------------------------------------------------
