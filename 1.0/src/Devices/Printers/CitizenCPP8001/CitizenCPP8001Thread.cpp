//---------------------------------------------------------------------------


#pragma hdrstop

#include "CitizenCPP8001Thread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

__fastcall CCitizenCPP8001Thread::CCitizenCPP8001Thread() : TDeviceThread(true, false)
{
  PollingMode = false;
  DataLength = 1;  
}

__fastcall CCitizenCPP8001Thread::~CCitizenCPP8001Thread()
{
}

