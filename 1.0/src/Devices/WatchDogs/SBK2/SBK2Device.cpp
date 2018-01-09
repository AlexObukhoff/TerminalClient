
#pragma hdrstop

#include "SBK2Device.h"
#include "globals.h"
#include "boost/format.hpp"

#pragma package(smart_init)

#define CommandTimeOut 60*1000

TSBK2Device::TSBK2Device(TLogClass* _Log) : TSBK2DeviceClass(_Log)
{
    hDll = NULL;
    WDTGetDoorSwitch = NULL;
    WDTSetTimer = NULL;
    WDTClearTimer = NULL;
    WDTStopTimer = NULL;
    WDTResetModem = NULL;
    WDTResetComputer = NULL;
    DeviceName = "SBK2";

    try
    {
        hDll = LoadLibrary("WDTS.dll");
        if(hDll)
        {
            try
            {
               WDTGetDoorSwitch  = (_WDTGetDoorSwitch)GetProcAddress (hDll, "WDTGetDoorSwitch");
            }
            catch (Exception &ex)
            {
                Log->Write((boost::format("WDTGetDoorSwitch error: %1%") % ex.Message.c_str()).str().c_str());
            }
            try
            {
                WDTSetTimer = (_WDTSetTimer)GetProcAddress (hDll, "WDTSetTimer");
            }
            catch (Exception &ex)
            {
                Log->Write((boost::format("WDTSetTimer error: %1%") % ex.Message.c_str()).str().c_str());
            }
            try
            {
                WDTClearTimer = (_WDTClearTimer)GetProcAddress (hDll, "WDTClearTimer");
            }
            catch (Exception &ex)
            {
                Log->Write((boost::format("WDTClearTimer error: %1%") % ex.Message.c_str()).str().c_str());
            }
            try
            {
                WDTResetComputer = (_WDTResetComputer)GetProcAddress (hDll, "WDTResetComputer");
            }
            catch (Exception &ex)
            {
                Log->Write((boost::format("WDTResetComputer error: %1%") % ex.Message.c_str()).str().c_str());
            }
            try
            {
                WDTStopTimer = (_WDTStopTimer)GetProcAddress (hDll, "WDTStopTimer");
            }
            catch (Exception &ex)
            {
                Log->Write((boost::format("WDTStopTimer error: %1%") % ex.Message.c_str()).str().c_str());
            }
            try
            {
                WDTResetModem = (_WDTResetModem)GetProcAddress (hDll, "WDTResetModem");
            }
            catch (Exception &ex)
            {
                Log->Write((boost::format("WDTResetModem error: %1%") % ex.Message.c_str()).str().c_str());
            }
        }
        else
        {
            Log->Write("Проверьте наличие DLL, библиотека WDTS.dll не найдена");
        }
    }
    catch (...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

TSBK2Device::~TSBK2Device()
{
    try
    {
        StopPooling();
        Sleep(200);
        if (hDll)
            FreeLibrary(hDll);
        Log->Write("~TSBK2Device()");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TSBK2Device::StartDevice()
{
  try
  {
      if (!GlobalStop)
        return;
      ClearAnswer();
      Stop();
      //создаём поток
      Start();
      if (DeviceThread)
      {
        DeviceThread->PollingMode = true;
        DeviceThread->OnlyPnP = OnlyPnP;
      }
      //запускаем поток
      Resume();
  }
  __finally
  {
  }
}

bool TSBK2Device::WaitForComplete(DWORD WaitTime)
{
    if (DeviceThread == NULL)
        return false;

    _info* Info = ((TSBK2DeviceThread*)DeviceThread)->ProcInfo;

    int interval = 10;
    DWORD SleepTime = CommandTimeOut;
    if (WaitTime > 0)
        SleepTime = WaitTime;

    while(Info->Status != Info->cs_Done)
    {
        Sleep(interval);
        SleepTime -= interval;
        Application->ProcessMessages();
        if (SleepTime <= 0)
            return false;
        if (DeviceThread == NULL)
            return false;
    }
    return true;
}

void TSBK2Device::ClearGSM()
{
    if (WDTSetTimer == NULL) return;
    SetExtCommand(EC_WDTResetModem);
    WaitForComplete();
}

void TSBK2Device::ResetPC()
{
    if (WDTSetTimer == NULL) return;
    SetExtCommand(EC_WDTResetComputer);
    WaitForComplete();
}

void TSBK2Device::StartTimer()
{
    if (WDTSetTimer == NULL) return;
    if (DeviceThread == NULL)
        StartDevice();
    SetExtCommand(EC_WDTSetTimer);
    WaitForComplete();
}

void TSBK2Device::StopTimer()
{
    if (WDTStopTimer == NULL) return;
    SetExtCommand(EC_WDTStopTimer);
    WaitForComplete();
}

bool TSBK2Device::IsInitialized()
{
    if(DeviceThread)
        return DeviceThread->IsInitialized();
    else
        return false;
}

bool TSBK2Device::IsItYou()
{
    return IsInitialized();
}

