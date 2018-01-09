//---------------------------------------------------------------------------


#pragma hdrstop

#include "CitizenPPU232Thread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall CCitizenPPU232Thread::CCitizenPPU232Thread() : TDeviceThread(true, false)
{
  PollingMode = false;
  DataLength = 1;
  LifeTime = 30000;
}

__fastcall CCitizenPPU232Thread::~CCitizenPPU232Thread()
{
}

