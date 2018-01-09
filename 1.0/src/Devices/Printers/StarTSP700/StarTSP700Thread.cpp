//---------------------------------------------------------------------------


#pragma hdrstop

#include "StarTSP700Thread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall CStarTSP700Thread::CStarTSP700Thread() : TDeviceThread(true, false)
{
  PollingMode = false;
  DataLength = 1;
}

__fastcall CStarTSP700Thread::~CStarTSP700Thread()
{
}

