//---------------------------------------------------------------------------


#pragma hdrstop

#include <math.h>
#include "KtekKeybDeviceThread.h"
#include "boost/format.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define PollingInterval 50

__fastcall TKtekKeybDeviceThread::TKtekKeybDeviceThread() : TDeviceThread(true)
{
  ParentWindow = NULL;
}

__fastcall TKtekKeybDeviceThread::~TKtekKeybDeviceThread()
{
}


void TKtekKeybDeviceThread::Poll()
{
  ClearBuffer(Answer);
  AnswerSize = ReadPort(Answer, AnswerSize);
  for(int i=0; i<AnswerSize; i++)
  {
    if (AnswerSize > 1)
      Log->Write((boost::format("ReadPort(size=%1%): Answer[%2%] =") % AnswerSize % i % GetByteExp(Answer[i])).str().c_str());
    if (Terminated) return;
    DeviceState->StateCode = Answer[i];
    CheckState(DeviceState->StateCode);
  }
}

void TKtekKeybDeviceThread::CheckState(BYTE code)
{
    AnsiString mess;
    if (Terminated) return;

    if (code == 0) return;

    DeviceState->StateCode = GetMapCode(code);
    DeviceState->StateDescription = GetByteExp(DeviceState->StateCode);
    Log->Write((boost::format("Key = %1%; MapKey = %2%") % GetByteExp(code) % GetByteExp(DeviceState->StateCode)).str().c_str());
    ChangeDeviceState();
    if (ParentWindow)
    {
        SendMessage(ParentWindow,WM_SETFOCUS,0,1);
        PostMessage(ParentWindow,WM_KEYDOWN,code,0);
        PostMessage(ParentWindow,WM_KEYUP,code,0);
    }
}

void __fastcall TKtekKeybDeviceThread::ProcessLoopCommand()
{
  ThreadTerminated = false;
  try
  {
    if (Terminated) return;
    SetInitialized();
    while(!Terminated)
    {
          int ticks = floor(PollingInterval/10);
          for(int i=0; i <= ticks; i++)
          {
            Sleep(10);
            if (Terminated) return;
          }
          if (Terminated) return;
          Poll();
    }
  }
  __finally
  {
  }
}

bool TKtekKeybDeviceThread::ChangeDeviceState(bool wait)
{
    DeviceState->StateChange = true;
    if (DeviceStateChanged)
      DeviceStateChanged(DeviceState);
    return true;
}

BYTE TKtekKeybDeviceThread::GetMapCode(BYTE value)
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
        case 0x1B://Отмена
            result =  VK_ESCAPE;
            break;
        case 0x08://Сброс
            result =  VK_BACK;
            break;
        case 0x0D://Ввод
            result =  VK_RETURN;
            break;
        case 0x10:
            result =  value;
            break;
        case 0x41://A left
            result =  VK_F1;
            break;
        case 0x42://B left
            result =  VK_F2;
            break;
        case 0x44://C left
            result =  VK_F3;
            break;
        case 0x43://D left
            result =  VK_F4;
            break;
        case 0x46://A right
            result =  VK_F5;
            break;
        case 0x45://B right
            result =  VK_F6;
            break;
        case 0x47://C right
            result =  VK_F7;
            break;
        case 0x48://D right
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
