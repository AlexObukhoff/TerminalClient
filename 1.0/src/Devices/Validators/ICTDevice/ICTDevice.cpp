
#pragma hdrstop

#include "ICTDevice.h"

#pragma package(smart_init)

TICTDevice::TICTDevice(int id, int ComPort,TLogClass* _Log) : TICTDeviceClass(id, ComPort,_Log)
{
  DataLength = 1;
  DeviceName = "ICTValidator";
  DeviceState->OutStateCode = DSE_NOTMOUNT;
}

TICTDevice::~TICTDevice()
{
}


