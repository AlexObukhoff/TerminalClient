#pragma hdrstop
#include "WatchDogAlarmDevice.h"
#include "WatchDogAlarmThread.h"
#include "globals.h"
#include "boost/format.hpp"
#pragma package(smart_init)

TWatchDogAlarmDevice::TWatchDogAlarmDevice(int ComPort,TLogClass* _Log) : TWatchDogAlarmDeviceClass(ComPort,_Log)
{
  DataLength = 1;
  LoggingErrors = false;
  Port->timeout = 250;
  DeviceName = "Alarm";
}

TWatchDogAlarmDevice::~TWatchDogAlarmDevice()
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

void TWatchDogAlarmDevice::StartTimer()
{
  if (!Port->PortInit)
  {
      Log->Write("Can't init COM port! Can't start device");
      return;
  }
  if (DeviceThread == NULL)
    StartDevice();
}

void TWatchDogAlarmDevice::StartDevice()
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

/*void TWatchDogAlarmDevice::SetCommand(BYTE command)
{
  if (DeviceThread == NULL)
    return;
  if (((TWatchDogAlarmDeviceThread*)DeviceThread)->_command != UNSET)
    for(int i = 1; i<=1000; i++)
    {
      Sleep(10);
      Application->ProcessMessages();
      if (((TWatchDogAlarmDeviceThread*)DeviceThread)->_command == UNSET)
        break;
    }
  DeviceThread->SetCommand(command);
}*/

void TWatchDogAlarmDevice::ClearGSM()
{
    SetExtCommand((BYTE)CLEARGSM);
}

void TWatchDogAlarmDevice::Lock()
{
    SetExtCommand((BYTE)LOCK);
}

void TWatchDogAlarmDevice::UnLock()
{
    SetExtCommand((BYTE)UNLOCK);
}

bool TWatchDogAlarmDevice::IsItYou()
{
    if (!Port->PortInit)
        return false;
        
    bool result = false;
    if (DeviceThread)
    {
        bool result1,result2 = false;
        SetExtCommand(STATE1);
        Sleep(600);
        if ((DeviceState)&&(DeviceState->AnswerSize >= 1))
            result1 = true;

        if (DeviceState)
            DeviceState->AnswerSize = 0;
        DeviceThread->ClearCOMPort();

        SetExtCommand(STATE2);
        Sleep(500);
        if ((DeviceState)&&(DeviceState->AnswerSize >= 1))
            result2 = true;
        if((result1)&&(result2))
            result = true;
    }
    return result;
}

clock_t TWatchDogAlarmDevice::GetCreationTime() //метка времени создания потока
{
    if (DeviceThread)
        return DeviceThread->CreationTime;
    else
        return -1;
}

