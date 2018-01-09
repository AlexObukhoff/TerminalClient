//---------------------------------------------------------------------------


#pragma hdrstop

#include "StarTUP900Thread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall CStarTUP900Thread::CStarTUP900Thread() : TDeviceThread(true, false)
{
  PollingMode = false;
  DataLength = 1;
}

__fastcall CStarTUP900Thread::~CStarTUP900Thread()
{
}

