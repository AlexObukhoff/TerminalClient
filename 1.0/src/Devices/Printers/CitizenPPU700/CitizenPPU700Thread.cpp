//---------------------------------------------------------------------------


#pragma hdrstop

#include "CitizenPPU700Thread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall CCitizenPPU700Thread::CCitizenPPU700Thread() : TDeviceThread(true, false)
{
  PollingMode = false;
  DataLength = 1;  
}

__fastcall CCitizenPPU700Thread::~CCitizenPPU700Thread()
{
}

