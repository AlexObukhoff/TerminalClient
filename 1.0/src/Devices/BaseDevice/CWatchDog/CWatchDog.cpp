//---------------------------------------------------------------------------


#pragma hdrstop

#include "CWatchDog.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

CWatchDog::CWatchDog(int ComPort,TLogClass* _Log, AnsiString Prefix)
: TDeviceClass(ComPort,_Log, Prefix)
{
  LoggingErrors = true;
  OnlyPnP = false;
  SIMnumber = 0;
  SIMmin = 1;
  SIMmax = 1;
  SIM_ChangingEnable = false;
}

CWatchDog::~CWatchDog()
{
}

bool CWatchDog::IsItYou()
{
    // для всех сторожевиков, которые не могут себя идентифицировать
    // мы будем считать, что они успешно инициализированны
    return true;
}

void CWatchDog::StartTimer()
{
}

void CWatchDog::StopTimer()
{
}

void CWatchDog::ClearGSM()
{
}

void CWatchDog::ResetPC()
{
}

void CWatchDog::StartGSM()
{
}

void CWatchDog::StopGSM()
{
}

void CWatchDog::UnLock()
{
}

void CWatchDog::Lock()
{
}

int CWatchDog::GetState()
{
}

void CWatchDog::GetSensors()
{
}

int CWatchDog::ChangeGSM(int SIMnumber)
{
    UNREFERENCED_PARAMETER(SIMnumber);
    return 0;
}

