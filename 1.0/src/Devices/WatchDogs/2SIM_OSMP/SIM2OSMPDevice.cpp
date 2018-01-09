
#pragma hdrstop

#include "SIM2OSMPdevice.h"
#include "SIM2OSMPThread.h"
#include "globals.h"
#include "boost/format.hpp"

#pragma package(smart_init)

TSIM2OSMPdevice::TSIM2OSMPdevice(int ComPort,TLogClass* _Log) : TSIM2OSMPDeviceClass(ComPort,_Log)
{
  DataLength = 1;
  LoggingErrors = false;
  Port->timeout = 500;
  DeviceName = "SIM2OSMP";
  SIMmax = 2;
  SIM_ChangingEnable = true;
}

TSIM2OSMPdevice::~TSIM2OSMPdevice()
{
  try
  {
    StopTimer();
    Sleep(500);
    StopPooling();
    Log->Write("~TSIM2OSMPdevice()");
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

void TSIM2OSMPdevice::StartTimer()
{
  if (!Port->PortInit)
  {
      Log->Write("Port init error! Can't start device");
      return;
  }
  if (DeviceThread == NULL)
    StartDevice();
}

void TSIM2OSMPdevice::StartDevice()
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
      DeviceThread->_PollingInterval = 30*1000;
      DeviceThread->OnlyPnP = OnlyPnP;
    }
    //запускаем поток
    Resume();
  }
  __finally
  {
  }
}

void TSIM2OSMPdevice::SetCommand(BYTE command)
{
  if (DeviceThread == NULL)
    return;
  if (((TSIM2OSMPDeviceThread*)DeviceThread)->ExtCommand != EXTC_Free)
    for(int i = 1; i<=1000; i++)
    {
      Sleep(10);
      Application->ProcessMessages();
      if (((TSIM2OSMPDeviceThread*)DeviceThread)->ExtCommand == EXTC_Free)
        break;
    }
  DeviceThread->SetCommand(command);
}

void TSIM2OSMPdevice::ClearGSM()
{
    SetExtCommand((BYTE)CLEARGSM);
}

void TSIM2OSMPdevice::ResetPC()
{
    SetExtCommand((BYTE)RESET);
}

void TSIM2OSMPdevice::StopTimer()
{
    SetExtCommand((BYTE)STOP);
}

int TSIM2OSMPdevice::ChangeGSM(int SIMnumber)
{
    ((TSIM2OSMPDeviceThread*)DeviceThread)->SIMnumber = 0;
    switch(SIMnumber)
    {
        case 1:
            SetExtCommand((BYTE)SIM1);
            break;

        case 2:
            SetExtCommand((BYTE)SIM2);
            break;
    }
    int ticks = 20;
    while( (((TSIM2OSMPDeviceThread*)DeviceThread)->SIMnumber == 0) &&(ticks > 0))
    {
        Sleep(10);
        Application->ProcessMessages();
        ticks--;
    }
    return ((TSIM2OSMPDeviceThread*)DeviceThread)->SIMnumber;
}

int TSIM2OSMPdevice::GetState()
{
    ((TSIM2OSMPDeviceThread*)DeviceThread)->SIMnumber = 0;
    SetExtCommand((BYTE)SIMSTATUS);
    int ticks = 20;
    while( (((TSIM2OSMPDeviceThread*)DeviceThread)->SIMnumber == 0)&&(ticks > 0))
    {
        Sleep(10);
        Application->ProcessMessages();
        ticks--;
    }
    return ((TSIM2OSMPDeviceThread*)DeviceThread)->SIMnumber;
}

bool TSIM2OSMPdevice::IsItYou()
{
    bool result = false;
    if ( (DeviceThread) && (((TSIM2OSMPDeviceThread*)DeviceThread)->Version == "WDT v1.00") && (((TSIM2OSMPDeviceThread*)DeviceThread)->VersionExt.IsEmpty() == false))
        result = true;
    return result;
}

clock_t TSIM2OSMPdevice::GetCreationTime() //метка времени создания потока
{
    if (DeviceThread)
        return DeviceThread->CreationTime;
    else
        return -1;
}

