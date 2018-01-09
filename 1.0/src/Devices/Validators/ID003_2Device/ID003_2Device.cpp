
#pragma hdrstop

#include "ID003_2device.h"

#pragma package(smart_init)

TID003_2device::TID003_2device(int id, int ComPort,TLogClass* _Log) : TID003_2DeviceClass(id, ComPort,_Log)
{
  DataLength = 0;
  DisableBill();
  DeviceName = "WBA003_2Validator";
  DeviceState->OutStateCode = DSE_NOTMOUNT;
}

TID003_2device::~TID003_2device()
{
  try
  {
    StopPooling();
  }
  __finally
  {
  }
}
