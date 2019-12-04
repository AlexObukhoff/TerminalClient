/* @file Инстанцирование шаблонов OPOS-сканеров. */

#include "../../../../modules/Hardware/Common/src/Base/DeviceBase.cpp"
#include "../../../../modules/Hardware/Common/src/Polling/PollingDeviceBase.cpp"
#include "../../../../modules/Hardware/Common/src/OPOS/OPOSPollingDeviceBase.cpp"

//------------------------------------------------------------------------------
template class DeviceBase<ProtoOPOSScanner>;
template class PollingDeviceBase<ProtoOPOSScanner>;
template class OPOSPollingDeviceBase<ProtoOPOSScanner, OPOS::OPOSScanner>;

//------------------------------------------------------------------------------
