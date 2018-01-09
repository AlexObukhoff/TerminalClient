#include <math.h>
//---------------------------------------------------------------------------
#pragma hdrstop
#include "WatchDogOSMPThread.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------

#pragma package(smart_init)

#define TIMEOUT 500

__fastcall TWatchDogOSMPDeviceThread::TWatchDogOSMPDeviceThread() : TDeviceThread(true)
{
  FreeOnTerminate = false;
  SendType = NotRecieveAnswer;
  DataLength = 1;
  Version = "";
  ExtCommand = EXTC_Free;
}

__fastcall TWatchDogOSMPDeviceThread::~TWatchDogOSMPDeviceThread()
{
  //SetCommand(STOP);
  //ProcessOutCommand();
  Log->Write("~TWatchDogOSMPDeviceThread()");
}

void TWatchDogOSMPDeviceThread::SendPacket(BYTE command)
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

void TWatchDogOSMPDeviceThread::GetVersion()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  DataLength = 9;
  Port->timeout = TIMEOUT;
  SendPacket(0x01);
  ProcessCommand();
  char version[10];
  memset(version,0,10);
  memcpy(version,Answer,9);
  Version = AnsiString(version);
  Log->Write((boost::format("Version = %1%") % Version.c_str()).str().c_str());

  DeviceState->StateDescription = "version request";
  if (Version == "WDT v1.00")
  {
    SetInitialized();
    if (DeviceState)
        DeviceState->OutStateCode = DSE_OK;
    ChangeDeviceState();
  }
}

void TWatchDogOSMPDeviceThread::StartTimer()
{
  if (!Port->PortInit)
    return;
  if (OnlyPnP) return;
  SendType = RecieveAnswer;
  DataLength = 1;
  Port->timeout = TIMEOUT;
  SendPacket(0x03);
  ProcessCommand();

  char str[10];
  memset(str,0,10);
  memcpy(str,Answer,1);
  DeviceState->SubStateDescription = str;
  DeviceState->StateDescription = "start timer";
  //if (DeviceStateChanged)
    //DeviceStateChanged(DeviceState);
}

void TWatchDogOSMPDeviceThread::StopTimer()
{
  if (!Port->PortInit)
    return;
  if (!Terminated)
    SendType = RecieveAnswer;
  else
    SendType = NotRecieveAnswer;
  DataLength = 1;
  Port->timeout = TIMEOUT;
  SendPacket(0x04);
  ProcessCommand();

  char str[10];
  memset(str,0,10);
  memcpy(str,Answer,1);
  DeviceState->SubStateDescription = str;
  DeviceState->StateDescription = "stop timer";
  //if (DeviceStateChanged)
    //DeviceStateChanged(DeviceState);
}

void TWatchDogOSMPDeviceThread::Life()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  DataLength = 1;
  Port->timeout = TIMEOUT;
  SendPacket(0x05);
  ProcessCommand();

  char str[10];
  memset(str,0,10);
  memcpy(str,Answer,1);
  DeviceState->SubStateDescription = str;
  DeviceState->StateDescription = "clear timer";
  //if (DeviceStateChanged)
    //DeviceStateChanged(DeviceState);
}

void TWatchDogOSMPDeviceThread::ClearGSM()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  DataLength = 1;
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

void TWatchDogOSMPDeviceThread::ResetPC()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  DataLength = 1;
  Port->timeout = TIMEOUT;
  SendPacket(0xAE);
  ProcessCommand();

  char str[10];
  memset(str,0,10);
  memcpy(str,Answer,1);
  DeviceState->SubStateDescription = str;
  DeviceState->StateDescription = "reset pc";
  //if (DeviceStateChanged)
    //DeviceStateChanged(DeviceState);
}

void TWatchDogOSMPDeviceThread::PollingLoop()
{
  if (!Port->PortInit)
    return;

  if (Terminated) return;

  GetVersion();
  StartTimer();

  while(!Terminated)
  {
    Life();
    //Application->ProcessMessages();
    ProcessOutCommand();
    int ticks = (int)ceill(_PollingInterval/10);
    for(int i = 1; i<=ticks; i++)
    {
      Sleep(10);
      //Application->ProcessMessages();
      ProcessOutCommand();
      if (Terminated)
        break;
    }
  }
  Log->Write("Exit from PollingLoop().");
  ProcessOutCommand();
}

void TWatchDogOSMPDeviceThread::SetCommand(BYTE command)
{
  switch ((TWatchDogOSMPCommand)command)
  {
    case STOP:
    case CLEARGSM:
    case VERSION:
    case RESET:
      ExtCommand = command;
      break;

    default:
      ExtCommand = EXTC_Free;
      break;
  }
}

void TWatchDogOSMPDeviceThread::ProcessOutCommand()
{
    //обновим метку времени, типа ещё жив поток
    CreationTime = clock();
    switch (ExtCommand)
    {
      case CLEARGSM:
        Log->Write("Clear GSM...");
        ClearGSM();
        break;
      case STOP:
        Log->Write("Stop timer...");
        StopTimer();
        break;
      case RESET:
        Log->Write("Reset PC...");
        ResetPC();
        break;
      case VERSION:
        Log->Write("Get Version");
        GetVersion();
        break;
    }
    ExtCommand = EXTC_Free;
}

void __fastcall TWatchDogOSMPDeviceThread::ProcessLoopCommand()
{
  PollingLoop();
}

void TWatchDogOSMPDeviceThread::ReadAnswer()
{
    AnswerSize = ReadPort(Answer,AnswerSize);
    DeviceState->StateCode = 0;
    /*for(int i=0; i<AnswerSize; i++)
    {
      switch(Answer[0])
      {
        case 0xA7:
          DeviceState->StateCode = 0xA7;
          if (ChangeEvent)
            ChangeEvent->Enabled = true;
          Log->Write("Door closed!");
          break;
        case 0xA8:
          DeviceState->StateCode = 0xA8;
          if (ChangeEvent)
            ChangeEvent->Enabled = true;
          Log->Write("Door opened!");
          break;
      }
    }*/
}

