/* @file Инстанцирование шаблонов диспенсеров. */

#include "Hardware/Dispensers/PortDispenser.h"

#include "../../../../modules/Hardware/CashDispensers/src/DispenserBase.cpp"
#include "../../../../modules/Hardware/CashDevices/src/CCTalk/CCTalkDeviceBase.cpp"

//------------------------------------------------------------------------------
template class DispenserBase<SerialDeviceBase<PortPollingDeviceBase<ProtoDispenser>>>;
template class CCTalkDeviceBase<TSerialDispenser>;
template class CCTalkDeviceBase<PortDispenser>;

//------------------------------------------------------------------------------
