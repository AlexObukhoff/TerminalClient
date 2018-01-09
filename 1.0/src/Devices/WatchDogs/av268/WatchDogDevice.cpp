
#pragma hdrstop

#include "WatchDogdevice.h"
#include "WatchDogThread.h"
#include "globals.h"
#include "boost/format.hpp"

#pragma package(smart_init)

TWatchDogdevice::TWatchDogdevice(int ComPort,TLogClass* _Log) : TWatchDogDeviceClass(ComPort,_Log)
{
  DataLength = 1;
  LoggingErrors = false;
  Port->timeout = 500;
  DeviceName = "st1.2";
}

TWatchDogdevice::~TWatchDogdevice()
{
  try
  {
    StopPooling();
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

void TWatchDogdevice::StartDevice()
{
  try
  {
    if (!GlobalStop)
      return;
    ClearAnswer();
    //StopThread();
    Stop();
    //создаём поток
    Start();
    if (DeviceThread)
    {
      DeviceThread->PollingMode = true;
      DeviceThread->_PollingInterval = 2000;
      DeviceThread->OnlyPnP = OnlyPnP;
    }
    //запускаем поток
    Resume();

    if (DeviceThread)
      ((TWatchDogDeviceThread*)DeviceThread)->Version = "";
    SetExtCommand((BYTE)VERSION);
  }
  __finally
  {
  }
}

void TWatchDogdevice::StartGSM()
{
  if (DeviceThread)
    DeviceThread->SetExtCommand((BYTE)STARTGSM);
}

void TWatchDogdevice::StopGSM()
{
  if (DeviceThread)
    DeviceThread->SetExtCommand((BYTE)STOPGSM);
}


void TWatchDogdevice::ClearGSM()
{
    StopGSM();
    Sleep(500);
    StartGSM();
}

void TWatchDogdevice::StopTimer()
{
  if (DeviceThread)
      DeviceThread->SetExtCommand((BYTE)MONITOROFF);
}

bool TWatchDogdevice::IsItYou()
{
    if (!Port->PortInit)
        return false;
    bool result = false;
    /*if (DeviceThread)
      ((TWatchDogDeviceThread*)DeviceThread)->Version = "";
    SetExtCommand((BYTE)VERSION);
    Sleep(500);*/
    if ((DeviceThread)&&(((TWatchDogDeviceThread*)DeviceThread)->Version != ""))
        result = true;
    return result;
}




