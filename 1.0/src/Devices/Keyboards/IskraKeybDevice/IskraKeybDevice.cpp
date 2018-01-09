
#pragma hdrstop

#include "IskraKeybDevice.h"

#pragma package(smart_init)

TIskraKeybDevice::TIskraKeybDevice(int id, int ComPort,TLogClass* _Log) : TIskraKeybDeviceClass(id, ComPort,_Log)
{
  DeviceName = "IskraKeyboard";
}

TIskraKeybDevice::~TIskraKeybDevice()
{
}


