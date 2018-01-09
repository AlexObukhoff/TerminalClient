/* @file Прото-устройства. */

#pragma once

// SDK
#include <SDK/Drivers/IPrinter.h>
#include <SDK/Drivers/IModem.h>

// Modules
#include "Hardware/Common/ProtoDevice.h"

//--------------------------------------------------------------------------------
class ProtoDeviceBase: virtual public SDK::Driver::IDevice, public ProtoDevice {};

class ProtoPrinter: public ProtoDevice, public SDK::Driver::IPrinter { SET_DEVICE_TYPE(Printer) };
class ProtoModem  : public ProtoDevice, public SDK::Driver::IModem   { SET_DEVICE_TYPE(Modem) };

//--------------------------------------------------------------------------------
