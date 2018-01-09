
#pragma hdrstop

#include "KtekKeybDevice.h"

#pragma package(smart_init)

TKtekKeybDevice::TKtekKeybDevice(int id, int ComPort,TLogClass* _Log) : TKtekKeybDeviceClass(id, ComPort,_Log)
{
  DeviceName = "KtekKeyboard";
}

TKtekKeybDevice::~TKtekKeybDevice()
{
}


