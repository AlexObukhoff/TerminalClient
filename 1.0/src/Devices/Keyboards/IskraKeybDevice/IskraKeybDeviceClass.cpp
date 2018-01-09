//---------------------------------------------------------------------------


#pragma hdrstop

#include "IskraKeybDeviceClass.h"
#include "IskraKeybDeviceThread.h"
#include "DeviceClass.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TIskraKeybDeviceClass::TIskraKeybDeviceClass(int id, int ComPort,TLogClass* _Log): CKeyboard(id, ComPort,_Log, "IskraKeybKeyboard")
{
  DataLength = 0;
  DataLengthIndex = -1;
  BeginByte = 0x02;
  EndByte = 0x03;
  CRCLength = 0;
  LoggingErrors = false;
  Port->timeout = 500;
}

TIskraKeybDeviceClass::~TIskraKeybDeviceClass()
{
}

void TIskraKeybDeviceClass::SetParentWindow(HWND value)
{
    _ParentWindow =  value;
    if (DeviceThread)
       ((TIskraKeybDeviceThread*)DeviceThread)->ParentWindow = _ParentWindow;
}

void TIskraKeybDeviceClass::Start()
{
  //включаем устройство, переход в режим ожидани€
  if (DeviceState->State != NotRun)
    return;
  DeviceState->State = Wait;
  DeviceThread = new TIskraKeybDeviceThread();
  DeviceThread->Log = Log;
  DeviceThread->DeviceState = DeviceState;
  DeviceThread->CommandParameters = CommandParameters;
  DeviceThread->SendType = SendType;
  DeviceThread->Port = Port;
  DeviceThread->Command = Command;
  DeviceThread->CommandSize = CommandSize;
  DeviceThread->Answer = Answer;
  DeviceThread->AnswerSize = AnswerSize;
  DeviceThread->data = data;
  DeviceThread->len_data = &len_data;

  DeviceThread->CommandCriticalSection = CommandCriticalSection;

  DeviceThread->DataLengthIndex = DataLengthIndex;
  DeviceThread->BeginByte = BeginByte;
  DeviceThread->LoggingErrors = LoggingErrors;
  DeviceThread->LastError = LastError;
  DeviceThread->EndByte = EndByte;
  DeviceThread->CRCLength = CRCLength;
  DeviceThread->DataLength = DataLength;

  DeviceThread->DeviceStarted = DeviceStarted;
  DeviceThread->DeviceStopped = DeviceStopped;
  DeviceThread->DevicePaused = DevicePaused;
  DeviceThread->DeviceStateChanged = DeviceStateChanged;
  DeviceThread->CommandStarted = CommandStarted;
  DeviceThread->CommandPaused = CommandPaused;
  DeviceThread->CommandFinished = CommandFinished;

  DeviceThread->ChangeEvent = ChangeEvent;
  DeviceState->Nominal = 0;
  ((TIskraKeybDeviceThread*)DeviceThread)->ParentWindow = ParentWindow;
}

bool TIskraKeybDeviceClass::IsItYou()
{
    int count = 1000;
    while(!SetExtCommand(EXTC_1)&&(count>0))
    {
        count--;
        Sleep(10);
    }
    Sleep(1000);
    if ((DeviceState)&&(DeviceState->OutStateCode == DSE_OK))
        return true;
    return false;
}

