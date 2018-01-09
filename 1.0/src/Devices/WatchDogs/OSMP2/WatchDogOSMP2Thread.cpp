#include <math.h>
//---------------------------------------------------------------------------
#pragma hdrstop
#include "WatchDogOSMP2Thread.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)

#define TIMEOUT 250

__fastcall TWatchDogOSMP2DeviceThread::TWatchDogOSMP2DeviceThread() : TDeviceThread(true)
{
  FreeOnTerminate = false;
  SendType = NotRecieveAnswer;
  LoggingErrors = false;
  DataLength = 4;
  Version = "";
  ExtCommand = EXTC_Free;
}

__fastcall TWatchDogOSMP2DeviceThread::~TWatchDogOSMP2DeviceThread()
{
  Log->Write("~TWatchDogOSMP2DeviceThread()");
}

void TWatchDogOSMP2DeviceThread::SendPacket(BYTE command)
{
  if (!Port->PortInit)
    return;
  try
  {
    //здесь в буфер пишем команду для выполнения
    ClearCommand();
    ClearAnswer();

    Command[0] = 0x4F;
    Command[1] = 0x53;
    Command[2] = 0x50;
    Command[3] = command;

    CommandParameters->SetParameters(Command,4,command,0);
    CommandSize = 4;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

void TWatchDogOSMP2DeviceThread::GetVersion()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  int OldDataLength = DataLength;
  DataLength = 9;
  Port->timeout = TIMEOUT;
  SendPacket(0x01);
  ProcessCommand();
  DataLength = OldDataLength;
  char version[10];
  memset(version,0,10);
  memcpy(version,Answer,9);
  Log->Write((boost::format("Version = %1%") % version).str().c_str());
  DeviceState->StateDescription = "version request";
  //if (DeviceStateChanged)
    //DeviceStateChanged(DeviceState);
}

void TWatchDogOSMP2DeviceThread::StartTimer()
{
  if (!Port->PortInit)
    return;
  if (OnlyPnP) return;
  SendType = RecieveAnswer;
  Port->timeout = TIMEOUT;
  int OldDataLength = DataLength;
  DataLength = 16;
  SendPacket(0x09);
  ProcessCommand();
  DataLength = OldDataLength;

  char str[10];
  memset(str,0,10);
  memcpy(str,Answer,1);
  DeviceState->SubStateDescription = str;
  DeviceState->StateDescription = "start timer";
  //if (DeviceStateChanged)
    //DeviceStateChanged(DeviceState);
}

void TWatchDogOSMP2DeviceThread::Life()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  Port->timeout = TIMEOUT;
  SendPacket(0x07);
  ProcessCommand();

  char str[10];
  memset(str,0,10);
  memcpy(str,Answer,1);
  DeviceState->SubStateDescription = str;
  DeviceState->StateDescription = "clear timer";
  //if (DeviceStateChanged)
    //DeviceStateChanged(DeviceState);
}

void TWatchDogOSMP2DeviceThread::ClearGSM()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  Port->timeout = TIMEOUT;
  SendPacket(0x02);
  ProcessCommand();

  char str[10];
  memset(str,0,10);
  memcpy(str,Answer,1);
  DeviceState->SubStateDescription = str;
  DeviceState->StateDescription = "clear gsm";
  //if (DeviceStateChanged)
    //DeviceStateChanged(DeviceState);
}

void TWatchDogOSMP2DeviceThread::PollingLoop()
{
  if (!Port->PortInit)
    return;

  if (Terminated) return;

  GetVersion();
  StartTimer();

  while(!Terminated)
  {
    Life();
    ProcessOutCommand();
    DWORD interval = _PollingInterval;
    int ticks = (int)ceill(interval/10);
    for(int i = 1; i<=ticks; i++)
    {
      Sleep(10);
      ProcessOutCommand();
      if (Terminated)
        break;
    }
  }
  Log->Write("Exit from PollingLoop().");
  ProcessOutCommand();
}

void TWatchDogOSMP2DeviceThread::ProcessOutCommand()
{
    //обновим метку времени, типа ещё жив поток
    CreationTime = clock();
    int Command = GetExtCommand();
    switch (Command)
    {
      case CLEARGSM:
        Log->Write("Clear GSM...");
        ClearGSM();
        break;
      case VERSION:
        Log->Write("Get Version");
        GetVersion();
        break;
    }
}

void __fastcall TWatchDogOSMP2DeviceThread::ProcessLoopCommand()
{
  PollingLoop();
}
