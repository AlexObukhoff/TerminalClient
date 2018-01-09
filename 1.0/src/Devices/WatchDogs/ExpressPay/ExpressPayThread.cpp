#include <math.h>
//---------------------------------------------------------------------------
#pragma hdrstop
#include "ExpressPayThread.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)

#define TIMEOUT 150

__fastcall TExpressPayDeviceThread::TExpressPayDeviceThread() : TDeviceThread(true)
{
  FreeOnTerminate = false;
  SendType = NotRecieveAnswer;
  LoggingErrors = false;
  DataLength = 1;
  Version = "";
  ExtCommand = EXTC_Free;
}

__fastcall TExpressPayDeviceThread::~TExpressPayDeviceThread()
{
  Log->Write("~TExpressPayDeviceThread()");
}

void TExpressPayDeviceThread::SendPacket(BYTE command)
{
  if (!Port->PortInit)
    return;
  try
  {
    //здесь в буфер пишем команду для выполнения
    ClearCommand();
    ClearAnswer();

    Command[0] = command;
//    Command[1] = 0x53;
//    Command[2] = 0x50;
//    Command[3] = command;

    CommandParameters->SetParameters(Command, 1, command, 0);
    CommandSize = 1;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

void TExpressPayDeviceThread::GetVersion()
{
  Log->Write("getVersion: - not supported");
  DeviceState->StateDescription = "version request";
}

void TExpressPayDeviceThread::StartTimer()
{
  DeviceState->StateDescription = "start timer";
}

void TExpressPayDeviceThread::Life()
{
  if (!Port->PortInit)
    return;

  SendType = NotRecieveAnswer;
  Port->timeout = TIMEOUT;
  SendPacket(0xFF);
  ProcessCommand();

  DeviceState->StateDescription = "clear timer";
}

void TExpressPayDeviceThread::ClearGSM()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  Port->timeout = TIMEOUT;
  SendPacket(0xFF ^ 0x10);
  ProcessCommand();

  DeviceState->StateDescription = "clear gsm";
}

void TExpressPayDeviceThread::PollingLoop()
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

void TExpressPayDeviceThread::ProcessOutCommand()
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

void __fastcall TExpressPayDeviceThread::ProcessLoopCommand()
{
  PollingLoop();
}
