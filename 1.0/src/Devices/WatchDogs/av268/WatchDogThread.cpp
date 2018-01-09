#include <math.h>
//---------------------------------------------------------------------------
#pragma hdrstop
#include "WatchDogThread.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)

__fastcall TWatchDogDeviceThread::TWatchDogDeviceThread() : TDeviceThread(true)
{
  SendType = NotRecieveAnswer;
  DataLength = 1;
  Version = "";
}

__fastcall TWatchDogDeviceThread::~TWatchDogDeviceThread()
{
}

void TWatchDogDeviceThread::SendPacket(BYTE command)
{
  if (!Port->PortInit)
    return;
  try
  {
    //здесь в буфер пишем команду для выполнения
    ClearCommand();
    ClearAnswer();

    Command[0] = command;
    CommandParameters->SetParameters(Command,1,command,0);
    CommandSize = 1;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TWatchDogDeviceThread::Start()
{
  if (!Port->PortInit)
    return;
  SendType = NotRecieveAnswer;
  SendPacket(0xAA);
  ProcessCommand();
}

void TWatchDogDeviceThread::Life()
{
  if (!Port->PortInit)
    return;
  SendType = NotRecieveAnswer;
  //SendPacket(0x5A);
  SendPacket(0xA5);
  ProcessCommand();
}

void TWatchDogDeviceThread::Pause()
{
  if (!Port->PortInit)
    return;
  SendType = NotRecieveAnswer;
  SendPacket(0xFA);
  ProcessCommand();
}

void TWatchDogDeviceThread::StartGSM()
{
  if (!Port->PortInit)
    return;
  SendType = NotRecieveAnswer;
  //SendPacket(0xAF);
  SendPacket(0x50);
  ProcessCommand();
}

void TWatchDogDeviceThread::StopGSM()
{
  if (!Port->PortInit)
    return;
  SendType = NotRecieveAnswer;
  //SendPacket(0x9F);
  SendPacket(0x60);
  ProcessCommand();
}

void TWatchDogDeviceThread::MonitorON()
{
  if (!Port->PortInit)
    return;

  if(OnlyPnP) return;

  SendType = NotRecieveAnswer;
  SendPacket(0x55);
  ProcessCommand();
}

void TWatchDogDeviceThread::MonitorOFF()
{
  if (!Port->PortInit)
    return;
  SendType = NotRecieveAnswer;
  SendPacket(0x65);
  ProcessCommand();
}

void TWatchDogDeviceThread::GetVersion()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  DataLength = 2;
  SendPacket(0x99);
  ProcessCommand();

  char version[3];
  memset(version,0,3);
  memcpy(version,Answer,2);
  Version = AnsiString(version);
  Log->Write((boost::format("Version = %1%") % version).str().c_str());
}

void TWatchDogDeviceThread::Delay5min()
{
  if (!Port->PortInit)
    return;
  SendType = NotRecieveAnswer;
  SendPacket(0xAA);
  ProcessCommand();
}

void TWatchDogDeviceThread::PollingLoop()
{
  if (!Port->PortInit)
    return;

  if (Terminated) return;

  GetVersion();

  MonitorON();
  //Start();
  while(!Terminated)
  {
    Life();
    ReadAnswer();
    ProcessOutCommand();
    int ticks = (int)ceill(_PollingInterval/10);
    for(int i = 1; i<=ticks; i++)
    {
      if (Terminated)
        continue;
      Sleep(10);
      ProcessOutCommand();
      ReadAnswer();
    }
  }
}

void TWatchDogDeviceThread::SetCommand(BYTE command)
{
  switch ((TWatchDogCommand)command)
  {
    case STARTGSM:
    case STOPGSM:
    case MONITORON:
    case MONITOROFF:
    case VERSION:
    case DELAY5MIN:
    case PAUSE:
      ExtCommand = command;
      break;

    default:
      ExtCommand = EXTC_Free;
      break;
  }
}

void TWatchDogDeviceThread::ProcessOutCommand()
{
    switch (ExtCommand)
    {
      case STARTGSM:
        Log->Write("Start GSM...");
        StartGSM();
        break;
      case STOPGSM:
        Log->Write("Stop GSM...");
        StopGSM();
        break;
      case MONITORON:
        Log->Write("Monitor On...");
        MonitorON();
        break;
      case MONITOROFF:
        Log->Write("Monitor Off...");
        MonitorOFF();
        break;
      case PAUSE:
        Log->Write("Pause...");
        Pause();
        break;
      case VERSION:
        Log->Write("Version...");
        GetVersion();
        break;
      case DELAY5MIN:
        Log->Write("Dalay 5 min...");
        Delay5min();
        break;
    }
    ExtCommand = EXTC_Free;
}

void __fastcall TWatchDogDeviceThread::ProcessLoopCommand()
{
    PollingLoop();
}

void TWatchDogDeviceThread::ReadAnswer()
{
    AnswerSize = ReadPort(Answer, AnswerSize);
    DeviceState->StateCode = 0;
    for(int i=0; i<AnswerSize; i++)
    {
      switch(Answer[i])
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
    }
}

