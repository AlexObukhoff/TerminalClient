//---------------------------------------------------------------------------


#pragma hdrstop

#include "WinPrnThread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall CWinPrinterThread::CWinPrinterThread() : TDeviceThread(true, false)
{
  PollingMode = false;
  DataLength = 1;
}

__fastcall CWinPrinterThread::~CWinPrinterThread()
{
}

