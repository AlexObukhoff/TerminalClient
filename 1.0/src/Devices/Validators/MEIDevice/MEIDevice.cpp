
#pragma hdrstop

#include "MEIdevice.h"

#pragma package(smart_init)

TMEIdevice::TMEIdevice(int id, int ComPort,TLogClass* _Log) : TMEIDeviceClass(id, ComPort,_Log)
{
  DataLength = 0;
  DisableBill();
  DeviceName = "MEIValidator";
  DeviceState->OutStateCode = DSE_NOTMOUNT;
}

TMEIdevice::~TMEIdevice()
{
  try
  {
    StopPooling();
  }
  __finally
  {
  }
}
