
#pragma hdrstop

#include "ID003_1device.h"

#pragma package(smart_init)

TID003_1device::TID003_1device(int id, int ComPort,TLogClass* _Log) : TID003_1DeviceClass(id, ComPort,_Log)
{
  DataLength = 0;
  DisableBill();
  DeviceName = "WBA003_1Validator";
  DeviceState->OutStateCode = DSE_NOTMOUNT;
}

TID003_1device::~TID003_1device()
{
  try
  {
    StopPooling();
  }
  __finally
  {
  }
}
