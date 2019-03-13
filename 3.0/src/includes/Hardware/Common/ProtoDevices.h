/* @file Прото-устройства. */

#pragma once

// SDK
#include <SDK/Drivers/IPrinter.h>
#include <SDK/Drivers/IModem.h>

// Modules
#include "Hardware/Common/ProtoDevice.h"

//--------------------------------------------------------------------------------
class ProtoDeviceBase: public ProtoDevice, public MetaDevice<SDK::Driver::IDevice> {};

class ProtoPrinter: public ProtoDevice, public MetaDevice<SDK::Driver::IPrinter> { SET_DEVICE_TYPE(Printer) };
class ProtoModem  : public ProtoDevice, public MetaDevice<SDK::Driver::IModem>   { SET_DEVICE_TYPE(Modem) };

//--------------------------------------------------------------------------------
