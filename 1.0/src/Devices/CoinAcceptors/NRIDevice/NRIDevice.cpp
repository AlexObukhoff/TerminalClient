
#pragma hdrstop

#include "NRIdevice.h"

#pragma package(smart_init)

TNRIdevice::TNRIdevice(int id, int ComPort,TLogClass* _Log) : TNRIDeviceClass(id, ComPort,_Log)
{
  DataLength = 0;
  Disable();
  DeviceName = "NRICoinAcceptor";
  DeviceState->OutStateCode = DSE_NOTMOUNT;
}

TNRIdevice::~TNRIdevice()
{
}
