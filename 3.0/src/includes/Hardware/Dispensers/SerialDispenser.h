#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"
#include "DispenserBase.h"

//--------------------------------------------------------------------------------
typedef DispenserBase<SerialDeviceBase<PortPollingDeviceBase<ProtoDispenser>>> TSerialDispenser;

//--------------------------------------------------------------------------------
