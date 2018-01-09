//---------------------------------------------------------------------------


#pragma hdrstop

#include "CitizenPPU231Thread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall CCitizenPPU231Thread::CCitizenPPU231Thread() : TDeviceThread(true, false)
{
  PollingMode = false;
  DataLength = 1;  
}

__fastcall CCitizenPPU231Thread::~CCitizenPPU231Thread()
{
}

