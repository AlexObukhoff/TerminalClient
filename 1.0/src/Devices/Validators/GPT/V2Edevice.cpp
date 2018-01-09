
#pragma hdrstop

#include "V2Edevice.h"

#pragma package(smart_init)

TV2Edevice::TV2Edevice(int id, int ComPort,TLogClass* _Log) : TV2EDeviceClass(id, ComPort,_Log)
{
  DataLength = 0;
  DisableBill();
  DeviceName = "V2EValidator";
  DeviceState->OutStateCode = DSE_NOTMOUNT;
}

TV2Edevice::~TV2Edevice()
{
}

