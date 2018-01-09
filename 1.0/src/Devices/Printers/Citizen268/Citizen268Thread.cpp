//---------------------------------------------------------------------------


#pragma hdrstop

#include "Citizen268Thread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall CCitizen268Thread::CCitizen268Thread() : TDeviceThread(true, false)
{
  PollingMode = false;
  DataLength = 1;  
}

__fastcall CCitizen268Thread::~CCitizen268Thread()
{
}

