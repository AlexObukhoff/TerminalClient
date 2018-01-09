
#pragma hdrstop

#include "CCNETdevice.h"
#include "CCNETDeviceThread.h"

#pragma package(smart_init)

TCCNETdevice::TCCNETdevice(int id, int ComPort,TLogClass* _Log) : TCCNETDeviceClass(id, ComPort,_Log)
{
  LoggingErrors = false;
  DisableBill();
  DeviceName = "CCNETValidator";
  DeviceState->OutStateCode = DSE_NOTMOUNT;
}

TCCNETdevice::~TCCNETdevice()
{
}
