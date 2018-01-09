#pragma hdrstop
#include "WatchDogOSMPdevice.h"
#include "WatchDogOSMPThread.h"
#include "globals.h"
#include "boost/format.hpp"
#pragma package(smart_init)

TWatchDogOSMPdevice::TWatchDogOSMPdevice(int ComPort,TLogClass* _Log) : TWatchDogOSMPDeviceClass(ComPort,_Log)
{
  DataLength = 1;
  LoggingErrors = false;
  Port->timeout = 500;
  DeviceName = "OSMP1";
}

TWatchDogOSMPdevice::~TWatchDogOSMPdevice()
{
  try
  {
    StopTimer();
    Sleep(500);
    StopPooling();
    Log->Write("~TWatchDogOSMPdevice()");
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

void TWatchDogOSMPdevice::StartTimer()
{
  if (!Port->PortInit)
  {
      Log->Write("Port int error! Can't start device");
      return;
  }
  if (DeviceThread == NULL)
    StartDevice();
}

void TWatchDogOSMPdevice::StartDevice()
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

void TWatchDogOSMPdevice::SetCommand(BYTE command)
{
  if (DeviceThread == NULL)
    return;
  if (((TWatchDogOSMPDeviceThread*)DeviceThread)->ExtCommand != EXTC_Free)
    for(int i = 1; i<=1000; i++)
    {
      Sleep(10);
      Application->ProcessMessages();
      if (((TWatchDogOSMPDeviceThread*)DeviceThread)->ExtCommand == EXTC_Free)
        break;
    }
  DeviceThread->SetCommand(command);
}

void TWatchDogOSMPdevice::ClearGSM()
{
    SetExtCommand((BYTE)CLEARGSM);
}

void TWatchDogOSMPdevice::ResetPC()
{
    SetExtCommand((BYTE)RESET);
}

void TWatchDogOSMPdevice::StopTimer()
{
    SetExtCommand((BYTE)STOP);
}

bool TWatchDogOSMPdevice::IsItYou()
{
    bool result = false;
    if (DeviceThread)
      ((TWatchDogOSMPDeviceThread*)DeviceThread)->Version = "";
    SetExtCommand((BYTE)VERSION);
    Sleep(500);
    if ( (DeviceThread) && (((TWatchDogOSMPDeviceThread*)DeviceThread)->Version == "WDT v1.00") )
        result = true;
    return result;
}

clock_t TWatchDogOSMPdevice::GetCreationTime() //метка времени создания потока
{
    if (DeviceThread)
        return DeviceThread->CreationTime;
    else
        return -1;
}

