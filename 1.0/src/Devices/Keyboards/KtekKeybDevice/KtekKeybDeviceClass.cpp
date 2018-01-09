//---------------------------------------------------------------------------


#pragma hdrstop

#include "KtekKeybDeviceClass.h"
#include "KtekKeybDeviceThread.h"
#include "DeviceClass.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TKtekKeybDeviceClass::TKtekKeybDeviceClass(int id, int ComPort,TLogClass* _Log): CKeyboard(id, ComPort,_Log, "KtekKeybKeyboard")
{
  DataLength = 0;
  DataLengthIndex = -1;
  BeginByte = 0x02;
  EndByte = 0x03;
  CRCLength = 0;
  LoggingErrors = false;
  Port->timeout = 500;
}

TKtekKeybDeviceClass::~TKtekKeybDeviceClass()
{
}

void TKtekKeybDeviceClass::SetParentWindow(HWND value)
{
    _ParentWindow =  value;
    if (DeviceThread)
       ((TKtekKeybDeviceThread*)DeviceThread)->ParentWindow = _ParentWindow;
}

void TKtekKeybDeviceClass::Start()
{
  //включаем устройство, переход в режим ожидани€
  if (DeviceState->State != NotRun)
    return;
  DeviceState->State = Wait;
  DeviceThread = new TKtekKeybDeviceThread();
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
  ((TKtekKeybDeviceThread*)DeviceThread)->ParentWindow = ParentWindow;
}

bool TKtekKeybDeviceClass::IsItYou()
{
    return false;
}

