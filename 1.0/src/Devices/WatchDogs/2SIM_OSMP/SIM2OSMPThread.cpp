#include <math.h>
//---------------------------------------------------------------------------


#pragma hdrstop

#include "SIM2OSMPThread.h"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define TIMEOUT 500

__fastcall TSIM2OSMPDeviceThread::TSIM2OSMPDeviceThread() : TDeviceThread(true)
{
  FreeOnTerminate = false;
  SendType = NotRecieveAnswer;
  DataLength = 1;
  Version = "";
  VersionExt = "";
  ExtCommand = EXTC_Free;
  SIMnumber = 0;
}

__fastcall TSIM2OSMPDeviceThread::~TSIM2OSMPDeviceThread()
{
  Log->Write("~TSIM2OSMPDeviceThread()");
}

void TSIM2OSMPDeviceThread::SendPacket(BYTE command)
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

void TSIM2OSMPDeviceThread::GetVersion()
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
  if (DeviceStateChanged)
    DeviceStateChanged(DeviceState);
}

bool TSIM2OSMPDeviceThread::GetVersionExt()
{
  VersionExt = "";
  if (!Port->PortInit)
    return false;
  SendType = RecieveAnswer;
  DataLength = 8;
  Port->timeout = TIMEOUT;
  SendPacket(0x09);
  ProcessCommand();
  char version[9];
  memset(version,0,9);
  memcpy(version,Answer,8);
  VersionExt = AnsiString(version);
  Log->Write((boost::format("Extended Version = %1%") % VersionExt.c_str()).str().c_str());

  DeviceState->StateDescription = "version ext request";
  if (DeviceStateChanged)
    DeviceStateChanged(DeviceState);

  if (VersionExt == "STODSIM")
      return true;
  else
      return false;
}

void TSIM2OSMPDeviceThread::StartTimer()
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
  if (DeviceStateChanged)
    DeviceStateChanged(DeviceState);
}

void TSIM2OSMPDeviceThread::StopTimer()
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
  if (DeviceStateChanged)
    DeviceStateChanged(DeviceState);
}

void TSIM2OSMPDeviceThread::Life()
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
  if (DeviceStateChanged)
    DeviceStateChanged(DeviceState);
}

void TSIM2OSMPDeviceThread::ClearGSM()
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
  if (DeviceStateChanged)
    DeviceStateChanged(DeviceState);
}

void TSIM2OSMPDeviceThread::ResetPC()
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
  if (DeviceStateChanged)
    DeviceStateChanged(DeviceState);
}

int TSIM2OSMPDeviceThread::ChangeSIM(int SIMnumber)
{
    if (!Port->PortInit)
      return SIMnumber = 0;
    SendType = RecieveAnswer;
    DataLength = 1;
    Port->timeout = TIMEOUT;

    switch(SIMnumber)
    {
        case 1:
            SendPacket(0x10);
            DeviceState->StateDescription = "Change to SIM1";
            break;

        case 2:
            SendPacket(0x11);
            DeviceState->StateDescription = "Change to SIM2";
            break;

        default:
            Log->Write((boost::format("Incorrect SIM number = %1%") % SIMnumber).str().c_str());
            return SIMnumber = 0;
    }

    ProcessCommand();

    Log->Write(DeviceState->StateDescription.c_str());
    if (DeviceStateChanged)
      DeviceStateChanged(DeviceState);

    SIMnumber = 0;
    return GetState();
}

int TSIM2OSMPDeviceThread::GetState()
{
    if (!Port->PortInit)
      return SIMnumber = 0;

    SendType = RecieveAnswer;
    DataLength = 1;
    Port->timeout = TIMEOUT;

    Port->ClearCOMPort();
    SendPacket(0x12);
    ProcessCommand();

    if ((DeviceState)&&(DeviceState->AnswerSize > 0))
        SIMnumber = Answer[0]+1;
    else
        SIMnumber = 0;

    DeviceState->StateDescription = "Get State...";
    Log->Write(DeviceState->StateDescription.c_str());
    Log->Write((boost::format("SIM number = %1%") % SIMnumber).str().c_str());
    if (DeviceStateChanged)
      DeviceStateChanged(DeviceState);

    return SIMnumber;
}

void TSIM2OSMPDeviceThread::Restart()
{
    if (!Port->PortInit)
      return;

    SendType = RecieveAnswer;
    DataLength = 1;
    Port->timeout = TIMEOUT;
    SendPacket(0x21);
    ProcessCommand();
    DeviceState->StateDescription = "Restart WatchDog";
    Log->Write(DeviceState->StateDescription.c_str());
    if (DeviceStateChanged)
      DeviceStateChanged(DeviceState);
}


void TSIM2OSMPDeviceThread::PollingLoop()
{
  if (!Port->PortInit)
    return;

  if (Terminated) return;

  GetVersion();

  if (GetVersionExt())
      Log->Write("2SIM Modem&WatchDog has been found.");
  else
      VersionExt = "";

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

void TSIM2OSMPDeviceThread::SetCommand(BYTE command)
{
  switch ((TSIM2OSMPCommand)command)
  {
    case STOP:
    case CLEARGSM:
    case VERSION:
    case RESET:
    case SIMSTATUS:
    case SIM1:
    case SIM2:
    case RESTART:
      ExtCommand = command;
      break;

    default:
      ExtCommand = EXTC_Free;
      break;
  }
}

void TSIM2OSMPDeviceThread::ProcessOutCommand()
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
      case RESTART:
        Log->Write("Restart watchDog");
        Restart();
        break;
      case SIM1:
        Log->Write("Change to SIM1");
        ChangeSIM(1);
        break;
      case SIM2:
        Log->Write("Change to SIM2");
        ChangeSIM(2);
        break;
      case SIMSTATUS:
        Log->Write("Get SIM status");
        GetState();
        break;
    }
    ExtCommand = EXTC_Free;
}

void __fastcall TSIM2OSMPDeviceThread::ProcessLoopCommand()
{
  PollingLoop();
}

void TSIM2OSMPDeviceThread::ReadAnswer()
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

