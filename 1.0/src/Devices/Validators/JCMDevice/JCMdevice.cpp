
#pragma hdrstop

#include "JCMdevice.h"
#include "JCMDeviceThread.h"

#pragma package(smart_init)

TJCMdevice::TJCMdevice(int id, int ComPort,TLogClass* _Log) : TJCMDeviceClass(id, ComPort,_Log)
{
  DataLength = 0;
  DisableBill();
  DeviceName = "JCMValidator";
  DeviceState->OutStateCode = DSE_NOTMOUNT;
}

TJCMdevice::~TJCMdevice()
{
}


