#pragma hdrstop
#include "WatchDogOSMP2device.h"
#pragma package(smart_init)
#include "WatchDogOSMP2Thread.h"
#include "globals.h"
#include "boost/format.hpp"

TWatchDogOSMP2device::TWatchDogOSMP2device(int ComPort,TLogClass* _Log) : TWatchDogOSMP2DeviceClass(ComPort,_Log)
{
  DataLength = 4;
  LoggingErrors = false;
  Port->timeout = 250;
  DeviceName = "OSMP2";
}

TWatchDogOSMP2device::~TWatchDogOSMP2device()
{
  try
  {
    StopTimer();
    Sleep(500);
    StopPooling();
    Log->Write("~TWatchDogOSMP2device()");
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

void TWatchDogOSMP2device::StartTimer()
{
  if (!Port->PortInit)
  {
      Log->Write("COM port init error! Can't start device");
      return;
  }
  if (DeviceThread == NULL)
    StartDevice();
}

void TWatchDogOSMP2device::StartDevice()
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
      DeviceThread->_PollingInterval = 2*1000;
      DeviceThread->OnlyPnP = OnlyPnP;
    }
    //запускаем поток
    Resume();
  }
  __finally
  {
  }
}

void TWatchDogOSMP2device::SetCommand(BYTE command)
{
  if (DeviceThread == NULL)
    return;
  if (((TWatchDogOSMP2DeviceThread*)DeviceThread)->ExtCommand != EXTC_Free)
    for(int i = 1; i<=1000; i++)
    {
      Sleep(10);
      Application->ProcessMessages();
      if (((TWatchDogOSMP2DeviceThread*)DeviceThread)->ExtCommand == EXTC_Free)
        break;
    }
  DeviceThread->SetCommand(command);
}

void TWatchDogOSMP2device::ClearGSM()
{
    SetExtCommand((BYTE)CLEARGSM);
}

void TWatchDogOSMP2device::ResetPC()
{
    SetExtCommand((BYTE)RESET);
}

void TWatchDogOSMP2device::StopTimer()
{
    SetExtCommand((BYTE)STOP);
}

bool TWatchDogOSMP2device::IsItYou()
{
    bool result = false;
    if (DeviceThread)
      ((TWatchDogOSMP2DeviceThread*)DeviceThread)->Version = "";
    SetExtCommand((BYTE)VERSION);
    Sleep(500);
    if (DeviceThread)
    {
        AnsiString version = ((TWatchDogOSMP2DeviceThread*)DeviceThread)->Version.UpperCase();
        if (version.Pos("WDT V2.00") > 0)
            result = true;
    }
    return result;
}

clock_t TWatchDogOSMP2device::GetCreationTime() //метка времени создания потока
{
    if (DeviceThread)
        return DeviceThread->CreationTime;
    else
        return -1;
}

