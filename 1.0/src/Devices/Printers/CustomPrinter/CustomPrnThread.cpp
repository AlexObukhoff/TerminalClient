//---------------------------------------------------------------------------


#pragma hdrstop

#include "CustomPrnThread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall CCustomPrnThread::CCustomPrnThread() : TDeviceThread(true, false)
{
  PollingMode = false;
  DataLength = 1;
}

__fastcall CCustomPrnThread::~CCustomPrnThread()
{
}

