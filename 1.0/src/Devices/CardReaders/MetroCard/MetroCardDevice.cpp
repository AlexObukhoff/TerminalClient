#pragma hdrstop
#include "MetroCardDevice.h"
#include "globals.h"
#include "boost/format.hpp"
#pragma package(smart_init)
#define CommandTimeOut 60*1000

TMetroCardDevice::TMetroCardDevice(int ComPort,TLogClass* _Log) : TMetroCardDeviceClass(ComPort,_Log)
{
    DeviceName = "MetroCardReader";
    hDll = NULL;
    ASKOPMInit = NULL;
    ASKOPMFindCard = NULL;
    ASKOPMGetMenu = NULL;
    ASKOPMWriteCard = NULL;

    try
    {
        hDll = LoadLibrary("ASKOPM_KIOSK.dll");
        if(hDll)
        {
            try
            {
                ASKOPMInit = (_ASKOPMInit)GetProcAddress (hDll, "_ASKOPMInit");
            }
            catch (Exception &ex)
            {
                Log->Write((boost::format(" initialization error: %1%") % ex.Message.c_str()).str().c_str());
            }
            try
            {
                ASKOPMFindCard = (_ASKOPMFindCard)GetProcAddress (hDll, "_ASKOPMFindCard");
            }
            catch (Exception &ex)
            {
                Log->Write((boost::format(" initialization error: %1%") % ex.Message.c_str()).str().c_str());
            }
            try
            {
                ASKOPMGetMenu = (_ASKOPMGetMenu)GetProcAddress (hDll, "_ASKOPMGetMenu");
            }
            catch (Exception &ex)
            {
                Log->Write((boost::format(" initialization error: %1%") % ex.Message.c_str()).str().c_str());
            }
            try
            {
                ASKOPMWriteCard = (_ASKOPMWriteCard)GetProcAddress (hDll, "_ASKOPMWriteCard");
            }
            catch (Exception &ex)
            {
                Log->Write((boost::format(" initialization error: %1%") % ex.Message.c_str()).str().c_str());
            }
        }
        else
        {
            Log->Write("DLL not found");
        }
    }
    catch (...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

TMetroCardDevice::~TMetroCardDevice()
{
    try
    {
        StopPooling();
        if (hDll)
            FreeLibrary(hDll);
        Log->Write("~TMetroCardDevice()");
    }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TMetroCardDevice::StartDevice()
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
        //DeviceThread->_PollingInterval = _PollingInterval;
      }
      //запускаем поток
      Resume();
  }
  __finally
  {
  }
}

bool TMetroCardDevice::IsItYou()
{
    return IsInitialized();
}

bool TMetroCardDevice::WaitForComplete(_info* Info, DWORD WaitTime)
{
    if ((Info == NULL)||(DeviceThread == NULL))
        return false;

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

int TMetroCardDevice::Init()
{
    if (InitInfo == NULL) return -1;
    SetExtCommand(EC_ASKOPMInit);
    bool result = WaitForComplete(InitInfo);
    if (result)
    {
        SetInitialized();
        return InitInfo->Result;
    }
    else
        return -1;
}

int TMetroCardDevice::FindCard()
{
    if (DeviceThread)
        ((TMetroCardDeviceThread*)DeviceThread)->CardFound = false;
    if (FindCardInfo == NULL) return -1;
    FindCardInfo->CardFound = false;
    SetExtCommand(EC_ASKOPMFindCard);
    /*bool result = WaitForComplete(FindCardInfo);
    if (result)
        return FindCardInfo->Result;
    else */
        return -1;

}

int TMetroCardDevice::GetMenu()
{
    if (GetMenuInfo == NULL) return -1;
    SetExtCommand(EC_ASKOPMGetMenu);
    bool result = WaitForComplete(GetMenuInfo);
    if (result)
        return GetMenuInfo->Result;
    else
        return -1;
}

int TMetroCardDevice::WriteCard()
{
    if (WriteCardInfo == NULL) return -1;
    SetExtCommand(EC_ASKOPMWriteCard);
    bool result = WaitForComplete(WriteCardInfo);
    if (result)
        return WriteCardInfo->Result;
    else
        return -1;
}

void TMetroCardDevice::StopOperation()
{
    SetExtCommand(EC_StopOperation);
}

void TMetroCardDevice::GetServerStatus(bool value)
{
    if (DeviceThread)
        ((TMetroCardDeviceThread*)DeviceThread)->ServerConnected = value;
}

bool TMetroCardDevice::SetServerStatus()
{
    if (DeviceThread)
        return ((TMetroCardDeviceThread*)DeviceThread)->ServerConnected;
    else
        return false;
}

