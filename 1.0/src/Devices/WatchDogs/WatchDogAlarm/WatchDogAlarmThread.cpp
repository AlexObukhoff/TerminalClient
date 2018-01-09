#include <math.h>
//---------------------------------------------------------------------------
#pragma hdrstop
#include "WatchDogAlarmThread.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#define TIMEOUT 250

__fastcall TWatchDogAlarmDeviceThread::TWatchDogAlarmDeviceThread() : TDeviceThread(true)
{
  FreeOnTerminate = false;
  SendType = NotRecieveAnswer;
  DataLength = 1;
}

__fastcall TWatchDogAlarmDeviceThread::~TWatchDogAlarmDeviceThread()
{
}

void TWatchDogAlarmDeviceThread::SendPacket(BYTE command)
{
  if (!Port->PortInit)
    return;
  try
  {
    //здесь в буфер пишем команду для выполнения
    ClearCommand();
    ClearAnswer();

    Command[0] = command;
    CommandSize = 1;
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
  }
}

void TWatchDogAlarmDeviceThread::StartTimer()
{
  if (!Port->PortInit)
    return;
  if (OnlyPnP) return;
  SendType = NotRecieveAnswer;
  DataLength = 1;
  Port->timeout = TIMEOUT;
  SendPacket(0x88);
  ProcessCommand();
  DeviceState->StateDescription = "start timer";
}

void TWatchDogAlarmDeviceThread::Life()
{
  if (!Port->PortInit)
    return;
  SendType = NotRecieveAnswer;
  DataLength = 1;
  Port->timeout = TIMEOUT;
  SendPacket(0x88);
  ProcessCommand();
  DeviceState->StateDescription = "clear timer";
}

void TWatchDogAlarmDeviceThread::ClearGSM()
{
  if (!Port->PortInit)
    return;
  SendType = NotRecieveAnswer;
  DataLength = 1;
  Port->timeout = TIMEOUT;
  SendPacket(0x84);
  ProcessCommand();
  DeviceState->StateDescription = "clear gsm";
}

void TWatchDogAlarmDeviceThread::Lock()
{
  if (!Port->PortInit)
    return;
  SendType = NotRecieveAnswer;
  DataLength = 1;
  Port->timeout = TIMEOUT;
  SendPacket(0x82);
  ProcessCommand();
  DeviceState->StateDescription = "lock";
}

void TWatchDogAlarmDeviceThread::UnLock()
{
  if (!Port->PortInit)
    return;
  SendType = NotRecieveAnswer;
  DataLength = 1;
  Port->timeout = TIMEOUT;
  SendPacket(0x81);
  ProcessCommand();
  DeviceState->StateDescription = "unlock";
}

void TWatchDogAlarmDeviceThread::GetState1()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  DataLength = 1;
  Port->timeout = TIMEOUT;
  SendPacket(0x80);
  ProcessCommand();
  DeviceState->StateDescription = "state1 request";
  DeviceState->StateCode = Answer[0];
}

void TWatchDogAlarmDeviceThread::GetState2()
{
  if (!Port->PortInit)
    return;
  SendType = RecieveAnswer;
  DataLength = 1;
  Port->timeout = TIMEOUT;
  SendPacket(0xC0);
  ProcessCommand();
  DeviceState->StateDescription = "state2 request";
  DeviceState->StateCode = Answer[0];
}

void TWatchDogAlarmDeviceThread::PollingLoop()
{
  if (!Port->PortInit)
    return;

  while(!Terminated)
  {
    Life();
    int period = 100;
    int ticks = (int)ceill(_PollingInterval/period);
    for(int i = 1; i<=ticks; i++)
    {
      ReadAnswer();
      Sleep(period);
      //Application->ProcessMessages();
      ProcessOutCommand();
      if (Terminated) break;
    }
  }
  Log->Write("Exit from PollingLoop().");
}

void TWatchDogAlarmDeviceThread::ProcessOutCommand()
{
    //обновим метку времени, типа ещё жив поток
    try
    {
        CreationTime = clock();
        switch (ExtCommand)
        {
          case CLEARGSM:
            Log->Write("Power Off modem");
            ClearGSM();
            break;
          case LOCK:
            Log->Write("Lock Door");
            Lock();
            break;
          case UNLOCK:
            Log->Write("UnLock Door");
            UnLock();
            break;
          case STATE1:
            Log->Write("Get State1");
            GetState1();
            break;
          case STATE2:
            Log->Write("Get State2");
            GetState2();
            break;
        }
    }
    __finally
    {
        //ExtCommand now is free for setting up new command
        ExtCommand = EXTC_Free;
    }
}

void __fastcall TWatchDogAlarmDeviceThread::ProcessLoopCommand()
{
  PollingLoop();
}

void TWatchDogAlarmDeviceThread::ReadAnswer()
{
    AnswerSize = ReadPort(Answer,AnswerSize);
    for(int i=0; i<AnswerSize; i++)
        CheckAnswer(Answer[i]);
}

void TWatchDogAlarmDeviceThread::CheckAnswer(BYTE value)
{
    DeviceState->OldStateCode = DeviceState->StateCode;
    DeviceState->StateCode = value;
    if (DeviceState->StateCode != DeviceState->OldStateCode)
    {
        Log->Write((boost::format("State = %1%; OldState = %2%;") % DeviceState->StateCode % DeviceState->OldStateCode).str().c_str());
        if ((value == 0x20)||(value == 0x40)||(value == 0x60))
        {
            switch(value)
            {
              case 0x20://наклон
                  //DeviceState->OutStateCode = DSE_MAINERROR;
                  DeviceState->SetOutCodes(DSE_NOERROR, WCD_INCLINATION);
                  DeviceState->SubStateDescription = "Наклон";
                  Log->Append(DeviceState->SubStateDescription.c_str());
                  break;
              case 0x40://удар
                  //DeviceState->OutStateCode = DSE_MAINERROR;
                  DeviceState->SetOutCodes(DSE_NOERROR, WCD_BEAT);
                  DeviceState->SubStateDescription = "Удар";
                  Log->Append(DeviceState->SubStateDescription.c_str());
                  break;
              case 0x60://наклон и удар
                  //DeviceState->OutStateCode = DSE_MAINERROR;
                  DeviceState->SetOutCodes(DSE_NOERROR, WCD_BEAT_AND_INCLINATION);
                  DeviceState->SubStateDescription = "Наклон и удар";
                  Log->Append(DeviceState->SubStateDescription.c_str());
                  break;
            }
            if (DeviceStateChanged)
                DeviceStateChanged(DeviceState);
        }
        else
        {
            if((value & 0x01) > 0)//сейф открыт
            {
                //DeviceState->OutStateCode = DSE_MAINERROR;
                DeviceState->SetOutCodes(DSE_NOERROR, WCD_BOX_OPENED);
                DeviceState->SubStateDescription = "Cейф открыт";
                Log->Append(DeviceState->SubStateDescription.c_str());
                if (DeviceStateChanged)
                    DeviceStateChanged(DeviceState);
            }
            else
            if((value & 0x02) > 0)//Верхний бокс открыт
            {
                //DeviceState->OutStateCode = DSE_MAINERROR;
                DeviceState->SetOutCodes(DSE_NOERROR, WCD_HIGH_BOX_OPENED);
                DeviceState->SubStateDescription = "Верхний бокс открыт";
                Log->Append(DeviceState->SubStateDescription.c_str());
                if (DeviceStateChanged)
                    DeviceStateChanged(DeviceState);
            }
            if((value & 0x04) > 0)//Нихний бокс открыт
            {
                //DeviceState->OutStateCode = DSE_MAINERROR;
                DeviceState->SetOutCodes(DSE_NOERROR, WCD_LOW_BOX_OPENED);
                DeviceState->SubStateDescription = "Верхний бокс открыт";
                Log->Append(DeviceState->SubStateDescription.c_str());
                if (DeviceStateChanged)
                    DeviceStateChanged(DeviceState);
            }
        }

    }
}

