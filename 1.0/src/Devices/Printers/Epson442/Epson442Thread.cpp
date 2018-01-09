//---------------------------------------------------------------------------


#pragma hdrstop

#include "Epson442Thread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall CEpson442Thread::CEpson442Thread() : TDeviceThread(true, false)
{
  PollingMode = false;
  DataLength = 1;
}

__fastcall CEpson442Thread::~CEpson442Thread()
{
}

