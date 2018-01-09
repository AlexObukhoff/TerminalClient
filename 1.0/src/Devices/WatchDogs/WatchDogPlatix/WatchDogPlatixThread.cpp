#include <math.h>
//---------------------------------------------------------------------------


#pragma hdrstop

#include "WatchDogPlatixThread.h"
#include "globals.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define TIMEOUT 250

__fastcall TWatchDogPlatixDeviceThread::TWatchDogPlatixDeviceThread() : TDeviceThread(true)
{
  FreeOnTerminate = false;
  SendType = RecieveAnswer;
  LoggingErrors = false;
  DataLengthIndex = 1;
  Version = "";
  ExtCommand = EXTC_Free;
}

__fastcall TWatchDogPlatixDeviceThread::~TWatchDogPlatixDeviceThread()
{
  Log->Write("~TWatchDogPlatixDeviceThread()");
}

void TWatchDogPlatixDeviceThread::SendPacket(BYTE command, BYTE* _data, int _datalen)
{
  if (!Port->PortInit)
    return;
  try
  {
    //здесь в буфер пишем команду для выполнения
    ClearCommand();
    ClearAnswer();

    Command[0] = 0xC0;
    Command[1] = 4 + _datalen;
    Command[2] = command;
    int pos = 3;
    if((_data != NULL)&&(_datalen != 0))
    {
        Move(_data,&Command[pos],_datalen);
        pos += _datalen;
    }
    BYTE CRC = Command[0];
    for(int i=1;i<pos; i++)
      CRC += Command[i];
    Command[pos] = CRC;

    CommandSize = pos+1;

    CommandParameters->SetParameters(Command,CommandSize,command,0);
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

void TWatchDogPlatixDeviceThread::ParseAnswer()
{
    try
    {
      ClearBuffer(data);
      *len_data = 0;
      if (DeviceState)
        AnswerSize = DeviceState->AnswerSize;
      if (AnswerSize>4)
      {
         int len_command = (int)Answer[1];
         if (len_command > 5)
         {
           *len_data = len_command - 4;
           memcpy(data,&Answer[3],*len_data);
         }
      }
    }
    __finally
    {
    }
}

void TWatchDogPlatixDeviceThread::GetVersion()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  SendPacket(0x11);
  ProcessCommand();
  char version[10];
  memset(version,0,10);
  memcpy(version,data,9);
  Log->Write((boost::format("Version = %1%") % version).str().c_str());
  DeviceState->StateDescription = "version request";
}

void TWatchDogPlatixDeviceThread::Reset(int number)
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  Port->timeout = TIMEOUT;
  switch (number)
  {
      case 0:
          SendPacket(0x25);
          DeviceState->StateDescription = "reset system";
          break;
      case 1:
          SendPacket(0x20);
          DeviceState->StateDescription = "reset output 1, clear gsm";
          break;
      case 2:
          SendPacket(0x21);
          DeviceState->StateDescription = "reset output 2";
          break;
      case 3:
          SendPacket(0x22);
          DeviceState->StateDescription = "reset output 3";
          break;
      case 4:
          SendPacket(0x23);
          DeviceState->StateDescription = "reset output 4";
          break;
      case 5:
          SendPacket(0x24);
          DeviceState->StateDescription = "reset output 5";//
          break;
  }
  ProcessCommand();
}

void TWatchDogPlatixDeviceThread::StartTimer()
{
  if (!Port->PortInit)
    return;
  if (OnlyPnP) return;
  SendType = RecieveAnswer;
  Port->timeout = TIMEOUT;
  SendPacket(0x13);
  ProcessCommand();
  DeviceState->StateDescription = "start timer";
}

void TWatchDogPlatixDeviceThread::StopTimer()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  Port->timeout = TIMEOUT;
  SendPacket(0x14);
  ProcessCommand();
  DeviceState->StateDescription = "stop timer";
}

void TWatchDogPlatixDeviceThread::Life()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  Port->timeout = TIMEOUT;
  SendPacket(0x26);
  ProcessCommand();
  DeviceState->StateDescription = "clear timer";
}

void TWatchDogPlatixDeviceThread::ClearGSM()
{
  if (!Port->PortInit)
    return;
  Reset(1);
}

void TWatchDogPlatixDeviceThread::PollingLoop()
{
  if (!Port->PortInit)
    return;

  if (Terminated) return;

  GetVersion();
  StartTimer();
  IsItYou();

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

void TWatchDogPlatixDeviceThread::ProcessOutCommand()
{
    //обновим метку времени, типа ещё жив поток
    CreationTime = clock();
    int Command = GetExtCommand();
    switch (Command)
    {
      case START:
        Log->Write("Start timer...");
        StartTimer();
        break;
      case STOP:
        Log->Write("Stop timer...");
        StopTimer();
        break;
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

void __fastcall TWatchDogPlatixDeviceThread::ProcessLoopCommand()
{
  PollingLoop();
}
