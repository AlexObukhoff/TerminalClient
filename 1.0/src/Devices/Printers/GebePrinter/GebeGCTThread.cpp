//---------------------------------------------------------------------------


#pragma hdrstop

#include "GebeGCTThread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall GebeGCTThread::GebeGCTThread() : TDeviceThread(true, false)
{
  PollingMode = false;
  DataLength = 1;
}

__fastcall GebeGCTThread::~GebeGCTThread()
{
}

