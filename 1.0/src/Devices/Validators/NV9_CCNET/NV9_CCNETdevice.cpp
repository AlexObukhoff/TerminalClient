
#pragma hdrstop

#include "NV9_CCNETdevice.h"
#include "NV9_CCNETDeviceThread.h"

#pragma package(smart_init)

TNV9_CCNETdevice::TNV9_CCNETdevice(int id, int ComPort,TLogClass* _Log) : TNV9_CCNETDeviceClass(id, ComPort,_Log)
{
  LoggingErrors = false;
  DisableBill();
  DeviceName = "NV9_CCNETValidator";
  DeviceState->OutStateCode = DSE_NOTMOUNT;
}

TNV9_CCNETdevice::~TNV9_CCNETdevice()
{
}
