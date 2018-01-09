//---------------------------------------------------------------------------
#pragma hdrstop
#include <math.h>
#include "IskraKeybDeviceThread.h"
#include "globals.h"
#include "boost/format.hpp"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#define PollingInterval 50

__fastcall TIskraKeybDeviceThread::TIskraKeybDeviceThread() : TDeviceThread(true)
{
  DataLength = 0;
  BeginByte = 0x02;
  EndByte = 0x03;
  CRCLength = 0;
  WriteReadTimeout = 50;
  CommandNumber = 0;
  LoggingTimeoutErrors = false;
  ParentWindow = NULL;
}

__fastcall TIskraKeybDeviceThread::~TIskraKeybDeviceThread()
{
}

BYTE TIskraKeybDeviceThread::GetCommandNumber()
{
    CommandNumber++;
    if (CommandNumber > 0x7F)
        CommandNumber = 0;
    return CommandNumber;
}

void TIskraKeybDeviceThread::SendPacket(BYTE command,int len_packet, BYTE* data)
{
    UNREFERENCED_PARAMETER(len_packet);
  if (!Port->PortInit)
  {
    DeviceState->StateCode = 0xff;
    return;
  }
  try
  {
    //здесь в буфер пишем команду дл€ выполнени€
    ClearCommand();
    ClearAnswer();

    Command[0] = 0x02;
    Command[1] = GetCommandNumber();
    Command[2] = command;
    if (data)
    {
        Command[3] = data[0];
        Command[4] = data[1];
    }
    else
    {
        Command[3] = 0;
        Command[4] = 0;
    }
    BYTE Result = Command[1];
    for(int i = 2; i<=4; i++)
        Result ^= Command[i];
    Command[5] = Result;
    Command[6] = 0x03;

    CommandSize = 7;
  }
  __finally
  {
  }
}

void TIskraKeybDeviceThread::GetStay()//POLL command
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    BYTE Data[2];
    Data[0] = 0xFF;
    //Data[0] = 0x00;
    Data[1] = 0x01;
    //BYTE command = (BYTE)"K";
    BYTE command = 0x4B;
    SendPacket(command,2,Data);
    SendType = RecieveAnswer;
    ProcessCommand();
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TIskraKeybDeviceThread::Reset()
{
  try
  {
    if (!Port->PortInit)
    {
      DeviceState->StateCode = 0xff;
      DeviceState->OutStateCode = DSE_NOTMOUNT;
      return;
    }
    //BYTE command = (BYTE)"I";
    BYTE command = 0x49;
    SendPacket(command);
    SendType = RecieveAnswer;
    ProcessCommand();
  }
    catch(...)
    {
        ExceptionFilter(__FILE__, BOOST_CURRENT_FUNCTION, __LINE__, Log);
    }
}

void TIskraKeybDeviceThread::ParseAnswer()
{
  try
  {
    if (DeviceState)
    {
        DeviceState->StateCode = 0;
        DeviceState->SubStateCode = 0;
        AnswerSize = DeviceState->AnswerSize;
        if (DeviceState->AnswerSize > 0)
        {
            DeviceState->StateCode = Answer[3];
        }
    }
  }
  __finally
  {
  }
}

void TIskraKeybDeviceThread::PollingLoop()
{
  AnsiString mess;

  if (!Port->PortInit)
  {
    DeviceState->StateCode = 0xff;
    return;
  }

  if (Terminated) return;

//============TEST=======================
  //Sleep(3000);
  //CheckState(GetMapCode(0x0D));
//=======================================

  GetStay();

  if((DeviceState)&&(DeviceState->AnswerSize <= 0))
  {
      if (DeviceState->OutStateCode != DSE_NOTMOUNT)
      {
          DeviceState->OutStateCode = DSE_NOTMOUNT;
          Log->Write("Keyboard is unmount.");
          DeviceState->StateCode = 0;
          ChangeDeviceState(false);
      }
      return;
  }
  else
  {
      if (DeviceState->OutStateCode != DSE_OK)
      {
          DeviceState->OutStateCode = DSE_OK;
          Log->Write("Keyboard is mount.");
          ParseAnswer();
          if (DeviceState->StateCode == 0)
          {
              ChangeDeviceState(false);
              return;
          }
          if (DeviceState->StateCode != 0)
          {
              BYTE MapCode = GetMapCode((BYTE)DeviceState->StateCode);
              CheckState(MapCode);
          }
          return;
      }
  }

  ParseAnswer();
  if (DeviceState->StateCode != 0)
  {
      BYTE MapCode = GetMapCode((BYTE)DeviceState->StateCode);
      CheckState(MapCode);
  }
}


void TIskraKeybDeviceThread::CheckState(BYTE code)
{
    AnsiString mess;
    if (Terminated) return;

    if (code == 0) return;

    Log->Write((boost::format("Key = %1%") % code).str().c_str());
    //DeviceState->OldStateCode = code;
    DeviceState->StateCode = code;
    ChangeDeviceState(false);
    if (ParentWindow)
    {
        SendMessage(ParentWindow,WM_SETFOCUS,0,1);
        PostMessage(ParentWindow,WM_KEYDOWN,code,0);
        PostMessage(ParentWindow,WM_KEYUP,code,0);
    }
}

void __fastcall TIskraKeybDeviceThread::ProcessLoopCommand()
{
  ThreadTerminated = false;
  try
  {
    if (Terminated) return;
    Log->Write("ProcessLoopCommand()");
    Reset();
    GetStay();
    if ((DeviceState)&&(DeviceState->AnswerSize <= 0))
    {
        DeviceState->OutStateCode = DSE_NOTMOUNT;
        if (Log != NULL)
          if (Log) Log->Write("No IskraKeyb Device Present!");
        DeviceState->StateChange = true;
        DeviceState->OldOutStateCode = DeviceState->OutStateCode;
        if (DeviceStateChanged)
          DeviceStateChanged(DeviceState);
    }
    else
    if ((DeviceState)&&(DeviceState->AnswerSize == 7))
    {
        DeviceState->OutStateCode = DSE_OK;
        if (Log != NULL)
          if (Log) Log->Write("IskraKeyb is OK.");
        DeviceState->StateChange = true;
        DeviceState->OldOutStateCode = DeviceState->OutStateCode;
        if (DeviceStateChanged)
          DeviceStateChanged(DeviceState);
    }

    SetInitialized();
    while(!Terminated)
    {
      int ticks = floor(PollingInterval/10);
      for(int i=0; i <= ticks; i++)
      {
        Sleep(10);
        //Application->ProcessMessages();
        if (Terminated) return;
      }
      if (Terminated) return;
      switch (ExtCommand)
      {
          case EXTC_1:
              GetStay();
              break;
          case EXTC_2:
              Reset();
              break;

          default:
              PollingLoop();
              break;
      }
      ExtCommand = EXTC_Free;
    }
  }
  __finally
  {
  }
}

bool TIskraKeybDeviceThread::ChangeDeviceState(bool wait)
{
    UNREFERENCED_PARAMETER(wait);
    DeviceState->StateChange = true;
    if (DeviceStateChanged)
      DeviceStateChanged(DeviceState);
    return true;
}

BYTE TIskraKeybDeviceThread::GetMapCode(BYTE value)
{
    BYTE result =  value;
    switch(value)
    {
        case 0x31://1
        case 0x32://2
        case 0x33://3
        case 0x34://4
        case 0x35://5
        case 0x36://6
        case 0x37://7
        case 0x38://8
        case 0x39://9
        case 0x30://0
            result =  value;
            break;
        case 0x2E://.
            result =  VK_DECIMAL;
            break;
        case 0x2C://,
            result =  VK_SEPARATOR;
            break;
        case 0x1B://ќтмена
            result =  VK_ESCAPE;
            break;
        case 0x08://—брос
            result =  VK_BACK;
            break;
        case 0x0D://¬вод
            result =  VK_RETURN;
            break;
        case 0x10:
            result =  value;
            break;
        case 0x11://A left
            result =  VK_F1;
            break;
        case 0x12://B left
            result =  VK_F2;
            break;
        case 0x13://C left
            result =  VK_F3;
            break;
        case 0x14://D left
            result =  VK_F4;
            break;
        case 0x15://A right
            result =  VK_F5;
            break;
        case 0x16://B right
            result =  VK_F6;
            break;
        case 0x17://C right
            result =  VK_F7;
            break;
        case 0x18://D right
            result =  VK_F8;
            break;
    }
    return result;
}

//SendMessage
/*
//SendMessage(HWND_BROADCAST,WM_SETFOCUS,0,1);
PostMessage(HWND_BROADCAST,WM_KEYDOWN,VK_RETURN,0);
PostMessage(HWND_BROADCAST,WM_KEYUP,VK_RETURN,0);
*/
