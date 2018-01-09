
#pragma hdrstop

#include "FairPayWDDevice.h"
#include "globals.h"
#include "boost/format.hpp"

#pragma package(smart_init)

#define CommandTimeOut 60*1000

TFairPayWDDevice::TFairPayWDDevice(TLogClass* _Log) : TFairPayWDDeviceClass(_Log)
{
    hDll = NULL;
    InitDevice     = NULL;
    DeInit         = NULL;
    StartWork      = NULL;
    SetValues      = NULL;
    ResetModem     = NULL;
    WriteIdleReport= NULL;
    DeviceName = "FairPay";


    try
    {
        hDll = LoadLibrary("hidwdt32.dll");
        if(hDll)
        {
            try
            {
               InitDevice  = (_InitDevice)GetProcAddress (hDll, "InitDevice");
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                Log->Write("InitDevice error");
            }
            try
            {
                DeInit = (_DeInit)GetProcAddress (hDll, "DeInit");
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                Log->Write("DeInit error");
            }
            try
            {
                StartWork = (_StartWork)GetProcAddress (hDll, "StartWork");
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                Log->Write("StartWork error");
            }
            try
            {
                SetValues = (_SetValues)GetProcAddress (hDll, "SetValues");
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                Log->Write("SetValues error");
            }
            try
            {
                ResetModem = (_ResetModem)GetProcAddress (hDll, "ResetModem");
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                Log->Write("ResetModem error");
            }
            try
            {
                WriteIdleReport = (_WriteIdleReport)GetProcAddress (hDll, "WriteIdleReport");
            }
            catch(...)
            {
                ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
                Log->Write("WriteIdleReport error");
            }
        }
        else
        {
            Log->Write("Library not found");
        }
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

TFairPayWDDevice::~TFairPayWDDevice()
{
    try
    {
        /*if (DeInit)
        {
            SetExtCommand(FP_DeInit);
            WaitForComplete();
            Sleep(500);
        }*/

        StopPooling();
        Sleep(200);
        if (hDll)
            FreeLibrary(hDll);
        Log->Write("~TFairPayWDDevice()");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TFairPayWDDevice::StartDevice()
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

bool TFairPayWDDevice::WaitForComplete(DWORD WaitTime)
{
    if (DeviceThread == NULL)
        return false;

    _info* Info = ((TFairPayWDDeviceThread*)DeviceThread)->ProcInfo;

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

void TFairPayWDDevice::ClearGSM()
{
    if (InitDevice == NULL) return;
    SetExtCommand(FP_ResetModem);
    WaitForComplete();
}


bool TFairPayWDDevice::IsInitialized()
{
    if(DeviceThread)
        return DeviceThread->IsInitialized();
    else
        return false;
}

bool TFairPayWDDevice::IsItYou()
{
    return IsInitialized();
}

void TFairPayWDDevice::StartTimer()
{
  if (!Port->PortInit)
  {
      Log->Write("Port init error! Can't start device");
      return;
  }
  if (DeviceThread == NULL)
    StartDevice();
}

