//---------------------------------------------------------------------------


#pragma hdrstop

#include "SBK2Class.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TSBK2DeviceClass::TSBK2DeviceClass(TLogClass* _Log) : CWatchDog(0,_Log, "SBK2")
{
    if (Port)
        delete Port;
    Port = NULL;
}

TSBK2DeviceClass::~TSBK2DeviceClass()
{
}

void TSBK2DeviceClass::Start()
{
  //включаем устройство, переход в режим ожидани€
  if (DeviceState->State != NotRun)
    return;
  DeviceState->State = Wait;
  DeviceThread = new TSBK2DeviceThread();
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
  DeviceThread->DisabledNominals = DisabledNominals;
  DeviceState->Nominal = 0;

  DeviceThread->CommandFinished = CommandFinished;

  //===================================================
  ((TSBK2DeviceThread*)DeviceThread)->hDll = hDll;

  ((TSBK2DeviceThread*)DeviceThread)->WDTGetDoorSwitch = WDTGetDoorSwitch;
  ((TSBK2DeviceThread*)DeviceThread)->WDTSetTimer = WDTSetTimer;
  ((TSBK2DeviceThread*)DeviceThread)->WDTClearTimer = WDTClearTimer;
  ((TSBK2DeviceThread*)DeviceThread)->WDTStopTimer = WDTStopTimer;
  ((TSBK2DeviceThread*)DeviceThread)->WDTResetModem = WDTResetModem;
  ((TSBK2DeviceThread*)DeviceThread)->WDTResetComputer = WDTResetComputer;
}

