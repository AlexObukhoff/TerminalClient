
#pragma hdrstop

#include "WatchDogPlatixDevice.h"

#pragma package(smart_init)

#include "WatchDogPlatixThread.h"
#include "globals.h"
#include "boost/format.hpp"

TWatchDogPlatixDevice::TWatchDogPlatixDevice(int ComPort,TLogClass* _Log) : TWatchDogPlatixDeviceClass(ComPort,_Log)
{
  DataLength = 4;
  LoggingErrors = false;
  Port->timeout = 250;
  DeviceName = "Platix";
}

TWatchDogPlatixDevice::~TWatchDogPlatixDevice()
{
  try
  {
    StopTimer();
    Sleep(500);
    StopPooling();
    Log->Write("~TWatchDogPlatixDevice()");
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

void TWatchDogPlatixDevice::StartTimer()
{
  if (!Port->PortInit)
  {
      Log->Write("Port init error! Can't start device");
      return;
  }
  if (DeviceThread == NULL)
    StartDevice();
}

void TWatchDogPlatixDevice::StartDevice()
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
      DeviceThread->_PollingInterval = 2*60*1000;
      DeviceThread->OnlyPnP = OnlyPnP;
    }
    //запускаем поток
    Resume();
  }
  __finally
  {
  }
}

/*void TWatchDogPlatixDevice::SetCommand(BYTE command)
{
  if (DeviceThread == NULL)
    return;
  if (((TWatchDogPlatixDeviceThread*)DeviceThread)->ExtCommand != EXTC_Free)
    for(int i = 1; i<=1000; i++)
    {
      Sleep(10);
      Application->ProcessMessages();
      if (((TWatchDogPlatixDeviceThread*)DeviceThread)->ExtCommand == EXTC_Free)
        break;
    }
  DeviceThread->SetCommand(command);
}*/

void TWatchDogPlatixDevice::ClearGSM()
{
    SetExtCommand((BYTE)CLEARGSM);
}

void TWatchDogPlatixDevice::ResetPC()
{
    SetExtCommand((BYTE)RESET);
}

void TWatchDogPlatixDevice::StopTimer()
{
    SetExtCommand((BYTE)STOP);
}

void TWatchDogPlatixDevice::GetVersion()
{
    SetExtCommand((BYTE)VERSION);
}

bool TWatchDogPlatixDevice::IsItYou()
{
    bool result = false;
    if (DeviceThread)
      ((TWatchDogPlatixDeviceThread*)DeviceThread)->Version = "";
    SetExtCommand((BYTE)VERSION);
    Sleep(500);
    if (DeviceThread)
    {
        char _pattern[3];
        _pattern[0] = 0xF1;
        _pattern[1] = 0xA7;
        _pattern[2] = 0x00;
        AnsiString pattern = AnsiString(_pattern);
        AnsiString version = ((TWatchDogPlatixDeviceThread*)DeviceThread)->Version.UpperCase();
        if (version == pattern)
            result = true;
        Log->Write((boost::format("pattern = %1%; version = %2%; result = %3%") % pattern.c_str() % version.c_str() % result).str().c_str());
    }
    return result;
}

clock_t TWatchDogPlatixDevice::GetCreationTime() //метка времени создания потока
{
    if (DeviceThread)
        return DeviceThread->CreationTime;
    else
        return -1;
}

